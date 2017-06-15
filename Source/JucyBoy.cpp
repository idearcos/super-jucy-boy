#include "JucyBoy.h"
#include <sstream>

JucyBoy::JucyBoy()
{
	setSize(160 * 4 + cpu_status_width_ + memory_map_width_ + ppu_tileset_width_, 144 * 4);
	setWantsKeyboardFocus(true);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	addChildComponent(audio_player_component_);

	cpu_debug_component_.addMouseListener(this, true);
	addAndMakeVisible(cpu_debug_component_);

	memory_debug_component_.addMouseListener(this, true);
	addAndMakeVisible(memory_debug_component_);

	memory_debug_component_.addMouseListener(this, true);
	addAndMakeVisible(ppu_debug_component_);

	// JucyBoy listeners
	listener_deregister_functions_.emplace_back(AddListener(cpu_debug_component_, &CpuDebugComponent::OnStatusUpdateRequested));
	listener_deregister_functions_.emplace_back(AddListener(memory_debug_component_, &MemoryDebugComponent::OnStatusUpdateRequested));
	listener_deregister_functions_.emplace_back(AddListener(ppu_debug_component_, &PpuDebugComponent::OnStatusUpdateRequested));

	// CPU listeners
	cpu_.CPU::AddListener(timer_);
	cpu_.CPU::AddListener(ppu_);
	cpu_.CPU::AddListener(apu_);
	cpu_.CPU::AddListener(*this);

	// Map memory read/write functions to MMU
	mmu_.MapMemoryRead(std::bind(&PPU::OnVramRead, &ppu_, std::placeholders::_1), Memory::Region::VRAM);
	mmu_.MapMemoryRead(std::bind(&PPU::OnOamRead, &ppu_, std::placeholders::_1), Memory::Region::OAM);
	mmu_.MapMemoryRead(std::bind(&CPU::OnInterruptsRead, &cpu_, std::placeholders::_1), Memory::Region::Interrupts);

	mmu_.MapMemoryWrite(std::bind(&PPU::OnVramWritten, &ppu_, std::placeholders::_1, std::placeholders::_2), Memory::Region::VRAM);
	mmu_.MapMemoryWrite(std::bind(&PPU::OnOamWritten, &ppu_, std::placeholders::_1, std::placeholders::_2), Memory::Region::OAM);
	mmu_.MapMemoryWrite(std::bind(&CPU::OnInterruptsWritten, &cpu_, std::placeholders::_1, std::placeholders::_2), Memory::Region::Interrupts);

	// Map IO register read/write functions to MMU
	mmu_.MapIoRegisterRead(std::bind(&CPU::OnIoMemoryRead, &cpu_, std::placeholders::_1), Memory::IF, Memory::IF);
	mmu_.MapIoRegisterRead(std::bind(&PPU::OnIoMemoryRead, &ppu_, std::placeholders::_1), Memory::LCDC, Memory::WX);
	mmu_.MapIoRegisterRead(std::bind(&APU::OnIoMemoryRead, &apu_, std::placeholders::_1), Memory::NR10, Memory::NR52);
	mmu_.MapIoRegisterRead(std::bind(&jb::Timer::OnIoMemoryRead, &timer_, std::placeholders::_1), Memory::DIV, Memory::TAC);
	mmu_.MapIoRegisterRead(std::bind(&Joypad::OnIoMemoryRead, &joypad_, std::placeholders::_1), Memory::JOYP, Memory::JOYP);

	mmu_.MapIoRegisterWrite(std::bind(&CPU::OnIoMemoryWritten, &cpu_, std::placeholders::_1, std::placeholders::_2), Memory::IF, Memory::IF);
	mmu_.MapIoRegisterWrite(std::bind(&PPU::OnIoMemoryWritten, &ppu_, std::placeholders::_1, std::placeholders::_2), Memory::LCDC, Memory::WX);
	mmu_.MapIoRegisterWrite(std::bind(&APU::OnIoMemoryWritten, &apu_, std::placeholders::_1, std::placeholders::_2), Memory::NR10, Memory::NR52);
	mmu_.MapIoRegisterWrite(std::bind(&jb::Timer::OnIoMemoryWritten, &timer_, std::placeholders::_1, std::placeholders::_2), Memory::DIV, Memory::TAC);
	mmu_.MapIoRegisterWrite(std::bind(&Joypad::OnIoMemoryWritten, &joypad_, std::placeholders::_1, std::placeholders::_2), Memory::JOYP, Memory::JOYP);

	// PPU listeners
	ppu_.AddListener(game_screen_component_);

	// APU listeners
	listener_deregister_functions_.emplace_back(apu_.AddListener([this](size_t right_sample, size_t left_sample) { audio_player_component_.OnNewSample(right_sample, left_sample); }));

	NotifyStatusUpdateRequest(false);
}

JucyBoy::~JucyBoy()
{
	cpu_.Stop();

	// Remove listeners
	cpu_.CPU::RemoveListener(timer_);
	cpu_.CPU::RemoveListener(ppu_);
	cpu_.CPU::RemoveListener(apu_);
	cpu_.CPU::RemoveListener(*this);

	ppu_.RemoveListener(game_screen_component_);

	for (auto deregister_function : listener_deregister_functions_)
	{
		deregister_function();
	}
	listener_deregister_functions_.clear();
}

void JucyBoy::Reset()
{
	// Stop execution, then reset all components
	cpu_.Stop();
	cpu_.Reset();
	mmu_.Reset();
	audio_player_component_.ClearBuffer();
}

void JucyBoy::LoadRom(const juce::File &file)
{
	Reset();
	cartridge_ = std::make_unique<Cartridge>(file.getFullPathName().toStdString());

	mmu_.MapMemoryRead(std::bind(&Cartridge::OnRomBank0Read, cartridge_.get(), std::placeholders::_1), Memory::Region::ROM_Bank0);
	mmu_.MapMemoryRead(std::bind(&Cartridge::OnRomBankNRead, cartridge_.get(), std::placeholders::_1), Memory::Region::ROM_OtherBanks);
	mmu_.MapMemoryRead(std::bind(&Cartridge::OnExternalRamRead, cartridge_.get(), std::placeholders::_1), Memory::Region::ERAM);

	mmu_.MapMemoryWrite(std::bind(&Cartridge::OnRomBank0Written, cartridge_.get(), std::placeholders::_1, std::placeholders::_2), Memory::Region::ROM_Bank0);
	mmu_.MapMemoryWrite(std::bind(&Cartridge::OnRomBankNWritten, cartridge_.get(), std::placeholders::_1, std::placeholders::_2), Memory::Region::ROM_OtherBanks);
	mmu_.MapMemoryWrite(std::bind(&Cartridge::OnExternalRamWritten, cartridge_.get(), std::placeholders::_1, std::placeholders::_2), Memory::Region::ERAM);

	NotifyStatusUpdateRequest(false);
}

void JucyBoy::paint (Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.setFont(14.0f);

	std::stringstream usage_instructions;
	usage_instructions << "Space: run / stop" << std::endl;
	usage_instructions << "Right: step over" << std::endl;
	g.drawFittedText(usage_instructions.str(), usage_instructions_area_, Justification::centred, 2);

	g.drawRect(usage_instructions_area_, 1);
}

void JucyBoy::resized()
{
	auto working_area = getLocalBounds();
	game_screen_component_.setBounds(working_area.removeFromLeft(160 * 4).removeFromTop(144 * 4));

	auto cpu_debug_area = working_area.removeFromLeft(cpu_status_width_);
	usage_instructions_area_ = cpu_debug_area.removeFromTop(40);
	cpu_debug_component_.setBounds(cpu_debug_area);

	auto memory_debug_area = working_area.removeFromLeft(memory_map_width_);
	memory_debug_component_.setBounds(memory_debug_area);

	auto ppu_tileset_area = working_area.removeFromLeft(ppu_tileset_width_);
	ppu_debug_component_.setBounds(ppu_tileset_area);
}

void JucyBoy::mouseDown( const MouseEvent &event)
{
	if (!event.mods.isRightButtonDown()) { return; }

	const auto was_cpu_running = cpu_.IsRunning();
	if (was_cpu_running) { cpu_.Stop(); }

	PopupMenu m;
	m.addItem(1, "Load ROM");
	const int result = m.show();
	
	switch (result)
	{
	case 0:
		// Did not select anything
		if (was_cpu_running) { cpu_.Run(); }
		break;
	case 1:
		{FileChooser rom_chooser{ "Select a ROM file to load...", File::getSpecialLocation(File::currentExecutableFile), "*.gb" };
		if (rom_chooser.browseForFileToOpen()) {
			auto rom_file = rom_chooser.getResult();
			try
			{
				LoadRom(rom_file);
				cpu_.Run();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", String{ "Error: " } +e.what());
			}
		}}
		break;
	default:
		break;
	}
}

bool JucyBoy::keyPressed(const KeyPress &key)
{
	// Switch statement does not work below because the keys are not compile time constants...
	if (key.getKeyCode() == KeyPress::spaceKey)
	{
		if (!cartridge_) { return true; }
		if (cpu_.IsRunning())
		{
			cpu_.Stop();
			NotifyStatusUpdateRequest(true);
		}
		else
		{
			cpu_.Run();
		}
	}
	else if (key.getKeyCode() == KeyPress::rightKey)
	{
		if (!cartridge_) { return true; }
		if (!cpu_.IsRunning())
		{
			try
			{
				cpu_.StepOver();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
			}
			NotifyStatusUpdateRequest(true);
		}
	}
	
	return true;
}

bool JucyBoy::keyStateChanged(bool /*isKeyDown*/)
{
	std::vector<Joypad::Keys> pressed_keys;

	if (KeyPress::isKeyCurrentlyDown('a'))
	{
		pressed_keys.push_back(Joypad::Keys::Left);
	}
	if (KeyPress::isKeyCurrentlyDown('s'))
	{
		pressed_keys.push_back(Joypad::Keys::Down);
	}
	if (KeyPress::isKeyCurrentlyDown('d'))
	{
		pressed_keys.push_back(Joypad::Keys::Right);
	}
	if (KeyPress::isKeyCurrentlyDown('w'))
	{
		pressed_keys.push_back(Joypad::Keys::Up);
	}
	if (KeyPress::isKeyCurrentlyDown('j'))
	{
		pressed_keys.push_back(Joypad::Keys::B);
	}
	if (KeyPress::isKeyCurrentlyDown('k'))
	{
		pressed_keys.push_back(Joypad::Keys::A);
	}
	if (KeyPress::isKeyCurrentlyDown(KeyPress::returnKey))
	{
		pressed_keys.push_back(Joypad::Keys::Start);
	}
	if (KeyPress::isKeyCurrentlyDown(KeyPress::tabKey))
	{
		pressed_keys.push_back(Joypad::Keys::Select);
	}

	joypad_.UpdatePressedKeys(pressed_keys);

	return true;
}

void JucyBoy::OnRunningLoopInterrupted()
{
	// The listener callback is called from within the CPU's running loop.
	// The call has to be forwarded to the message thread in order to join the running loop thread.
	// Moreover, any update to the GUI components (as the listener callback of Reset) can only be done safely in the message thread.
	triggerAsyncUpdate();
}

void JucyBoy::handleAsyncUpdate()
{
	NotifyStatusUpdateRequest(true);

	try
	{
		// Join the thread. If an exception was thrown in the running loop, Stop will rethrow it.
		cpu_.Stop();
	}
	catch (std::exception &e)
	{
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
	}
}

void JucyBoy::NotifyStatusUpdateRequest(bool compute_diff)
{
	for (auto& listener : listeners_)
	{
		listener(compute_diff);
	}
}
