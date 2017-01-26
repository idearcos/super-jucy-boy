#include "JucyBoy.h"
#include <sstream>

JucyBoy::JucyBoy()
{
	setSize (160 * 4 + cpu_status_width_ + memory_map_width_, 144 * 4);
	setWantsKeyboardFocus(true);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	cpu_debug_component_.addMouseListener(this, true);
	addAndMakeVisible(cpu_debug_component_);

	memory_debug_component_.addMouseListener(this, true);
	addAndMakeVisible(memory_debug_component_);

	// Add listeners
	listener_deregister_functions_.emplace_back(AddListener(cpu_debug_component_, &CpuDebugComponent::OnStatusUpdateRequested));
	listener_deregister_functions_.emplace_back(AddListener(memory_debug_component_, &MemoryDebugComponent::OnStatusUpdateRequested));

	cpu_.AddListener(timer_);
	cpu_.AddListener(gpu_);
	cpu_.AddListener(oam_dma_);
	cpu_.AddListener(*this);

	listener_deregister_functions_.emplace_back(mmu_.AddListener(cpu_, &CPU::OnIoMemoryWritten, Memory::Region::IO));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(cpu_, &CPU::OnInterruptsRegisterWritten, Memory::Region::Interrupts));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnVramWritten, Memory::Region::VRAM));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnOamWritten, Memory::Region::OAM));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(gpu_, &GPU::OnIoMemoryWritten, Memory::Region::IO));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(timer_, &jb::Timer::OnIoMemoryWritten, Memory::Region::IO));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(joypad_, &Joypad::OnIoMemoryWritten, Memory::Region::IO));
	listener_deregister_functions_.emplace_back(mmu_.AddListener(oam_dma_, &OamDma::OnIoMemoryWritten, Memory::Region::IO));

	gpu_.AddListener(game_screen_component_);

	NotifyStatusUpdateRequest(false);
}

JucyBoy::~JucyBoy()
{
	cpu_.Stop();

	// Remove listeners
	cpu_.RemoveListener(gpu_);
	cpu_.RemoveListener(*this);
	gpu_.RemoveListener(game_screen_component_);

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
}

void JucyBoy::LoadRom(const juce::File &file)
{
	Reset();

	// Convert the juce file to std string
	mmu_.LoadRom(file.getFullPathName().toStdString());

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
	usage_instructions_area_ = cpu_debug_area.removeFromTop(getHeight() / 10);
	cpu_debug_component_.setBounds(cpu_debug_area);

	auto memory_debug_area = working_area.removeFromLeft(memory_map_width_);
	memory_debug_component_.setBounds(memory_debug_area);
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
		if (!mmu_.IsRomLoaded()) { return true; }
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
		if (!mmu_.IsRomLoaded()) { return true; }
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
