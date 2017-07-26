#include "JucyBoy.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include "cereal/archives/binary.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

JucyBoy::JucyBoy()
{
	setSize(ComputeWindowWidth(), 144 * 4);
	setWantsKeyboardFocus(true);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	addChildComponent(audio_player_component_);
}

JucyBoy::~JucyBoy()
{
	PauseEmulation();
}

void JucyBoy::LoadRom(std::string file_path)
{
	audio_player_component_.ClearBuffer();

	// Create all JucyBoy classes and debug components
	mmu_ = std::make_unique<MMU>();
	cpu_ = std::make_unique<DebugCPU>(*mmu_);
	ppu_ = std::make_unique<DebugPPU>(*mmu_);
	apu_ = std::make_unique<APU>();
	timer_ = std::make_unique<jb::Timer>(*mmu_);
	joypad_ = std::make_unique<Joypad>();
	cartridge_ = std::make_unique<Cartridge>(file_path);

	// Register all listener functions
	// CPU listeners
	cpu_->CPU::AddListener(*timer_);
	cpu_->CPU::AddListener(*ppu_);
	cpu_->CPU::AddListener(*apu_);
	cpu_->CPU::AddListener(*this);

	// Map memory read/write functions to MMU
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return ppu_->OnVramRead(relative_address); }, Memory::Region::VRAM);
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return ppu_->OnOamRead(relative_address); }, Memory::Region::OAM);
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return cpu_->OnInterruptsRead(relative_address); }, Memory::Region::Interrupts);

	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { ppu_->OnVramWritten(relative_address, value); }, Memory::Region::VRAM);
	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { ppu_->OnOamWritten(relative_address, value); }, Memory::Region::OAM);
	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cpu_->OnInterruptsWritten(relative_address, value); }, Memory::Region::Interrupts);

	// Map IO register read/write functions to MMU
	mmu_->MapIoRegisterRead([this](Memory::Address relative_address) { return cpu_->OnIoMemoryRead(relative_address); }, Memory::IF, Memory::IF);
	mmu_->MapIoRegisterRead([this](Memory::Address relative_address) { return ppu_->OnIoMemoryRead(relative_address); }, Memory::LCDC, Memory::WX);
	mmu_->MapIoRegisterRead([this](Memory::Address relative_address) { return apu_->OnIoMemoryRead(relative_address); }, Memory::NR10, Memory::NR52);
	mmu_->MapIoRegisterRead([this](Memory::Address relative_address) { return timer_->OnIoMemoryRead(relative_address); }, Memory::DIV, Memory::TAC);
	mmu_->MapIoRegisterRead([this](Memory::Address relative_address) { return joypad_->OnIoMemoryRead(relative_address); }, Memory::JOYP, Memory::JOYP);
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return cartridge_->OnRomBank0Read(relative_address); }, Memory::Region::ROM_Bank0);
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return cartridge_->OnRomBankNRead(relative_address); }, Memory::Region::ROM_OtherBanks);
	mmu_->MapMemoryRead([this](Memory::Address relative_address) { return cartridge_->OnExternalRamRead(relative_address); }, Memory::Region::ERAM);

	mmu_->MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { cpu_->OnIoMemoryWritten(relative_address, value); }, Memory::IF, Memory::IF);
	mmu_->MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { ppu_->OnIoMemoryWritten(relative_address, value); }, Memory::LCDC, Memory::WX);
	mmu_->MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { apu_->OnIoMemoryWritten(relative_address, value); }, Memory::NR10, Memory::NR52);
	mmu_->MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { timer_->OnIoMemoryWritten(relative_address, value); }, Memory::DIV, Memory::TAC);
	mmu_->MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { joypad_->OnIoMemoryWritten(relative_address, value); }, Memory::JOYP, Memory::JOYP);
	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_->OnRomBank0Written(relative_address, value); }, Memory::Region::ROM_Bank0);
	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_->OnRomBankNWritten(relative_address, value); }, Memory::Region::ROM_OtherBanks);
	mmu_->MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_->OnExternalRamWritten(relative_address, value); }, Memory::Region::ERAM);

	// PPU listeners
	ppu_->AddListener(game_screen_component_);

	// APU listeners
	listener_deregister_functions_.emplace_back(apu_->AddListener([this](size_t right_sample, size_t left_sample) { audio_player_component_.OnNewSample(right_sample, left_sample); }));

	loaded_rom_file_path_ = std::move(file_path);

	ConstructDebugComponents();
}

void JucyBoy::ConstructDebugComponents()
{
	assert(cpu_ && mmu_ && ppu_);

	if (cpu_debug_component_) removeChildComponent(cpu_debug_component_.get());
	cpu_debug_component_ = std::make_unique<CpuDebugComponent>(*cpu_);
	cpu_debug_component_->addMouseListener(this, true);
	addChildComponent(*cpu_debug_component_);
	EnableDebugging(*cpu_debug_component_, false);

	if (memory_map_component_) removeChildComponent(memory_map_component_.get());
	memory_map_component_ = std::make_unique<MemoryMapComponent>(*mmu_);
	memory_map_component_->addMouseListener(this, true);
	addChildComponent(*memory_map_component_);
	EnableDebugging(*memory_map_component_, false);

	if (ppu_debug_component_) removeChildComponent(ppu_debug_component_.get());
	ppu_debug_component_ = std::make_unique<PpuDebugComponent>(*ppu_);
	ppu_debug_component_->addMouseListener(this, true);
	addChildComponent(*ppu_debug_component_);
	EnableDebugging(*ppu_debug_component_, false);
}

void JucyBoy::StartEmulation()
{
	assert(cpu_debug_component_ && memory_map_component_ && ppu_debug_component_);

	if (cpu_ && !cpu_->IsRunning())
	{
		cpu_debug_component_->isVisible() ? cpu_->DebugRun() : cpu_->Run();
	}

	cpu_debug_component_->OnEmulationStarted();
	memory_map_component_->OnEmulationStarted();
}

void JucyBoy::PauseEmulation()
{
	if (!cpu_) return;

	assert(cpu_debug_component_ && memory_map_component_ && ppu_debug_component_);

	try
	{
		// Join the thread. If an exception was thrown in the running loop, Stop will rethrow it.
		cpu_->Stop();
	}
	catch (std::exception &e)
	{
		AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
	}

	cpu_debug_component_->OnEmulationPaused();
	memory_map_component_->OnEmulationPaused();
}

void JucyBoy::UpdateDebugComponents(bool compute_diff)
{
	cpu_debug_component_->UpdateState(compute_diff);
	memory_map_component_->UpdateState(compute_diff);
	ppu_debug_component_->UpdateState();
}

void JucyBoy::paint(Graphics& g)
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

	if (cpu_debug_component_ && cpu_debug_component_->isVisible())
	{
		auto cpu_debug_area = working_area.removeFromLeft(cpu_status_width_);
		usage_instructions_area_ = cpu_debug_area.removeFromTop(40);
		cpu_debug_component_->setBounds(cpu_debug_area);
	}

	if (memory_map_component_ && memory_map_component_->isVisible())
	{
		auto memory_debug_area = working_area.removeFromLeft(memory_map_width_);
		memory_map_component_->setBounds(memory_debug_area);
	}

	if (ppu_debug_component_ && ppu_debug_component_->isVisible())
	{
		auto ppu_tileset_area = working_area.removeFromLeft(ppu_tileset_width_);
		ppu_debug_component_->setBounds(ppu_tileset_area);
	}
}

void JucyBoy::SaveState() const
{
	auto save_state_file_path{ loaded_rom_file_path_ };
	const auto last_dot_position = save_state_file_path.find_last_of('.');
	if (std::string::npos != last_dot_position)
	{
		save_state_file_path.replace(last_dot_position, std::string::npos, ".sav");
	}
	else
	{
		save_state_file_path.append(".sav");
	}

	std::ofstream save_state_file{ save_state_file_path, std::ios::binary };

	{cereal::BinaryOutputArchive  output_archive{ save_state_file };
	output_archive(*cpu_, *mmu_, *ppu_, *apu_, *timer_, *joypad_, *cartridge_); }

	save_state_file.close();
}

void JucyBoy::LoadState()
{
	auto save_state_file_path{ loaded_rom_file_path_ };
	const auto last_dot_position = save_state_file_path.find_last_of('.');
	if (std::string::npos != last_dot_position)
	{
		save_state_file_path.replace(last_dot_position, std::string::npos, ".sav");
	}
	else
	{
		save_state_file_path.append(".sav");
	}

	std::ifstream save_state_file{ save_state_file_path, std::ios::binary };

	{cereal::BinaryInputArchive  input_archive{ save_state_file };
	input_archive(*cpu_, *mmu_, *ppu_, *apu_, *timer_, *joypad_, *cartridge_); }

	save_state_file.close();
}

void JucyBoy::mouseDown(const MouseEvent &event)
{
	if (!event.mods.isRightButtonDown()) { return; }

	bool was_cpu_running{ cpu_ ? cpu_->IsRunning() : false };
	if (was_cpu_running)
	{
		PauseEmulation();
		UpdateDebugComponents(false);
	}

	PopupMenu m;
	int item_index{ 0 };
	m.addItem(++item_index, "Load ROM");
	m.addItem(++item_index, "Reset", cpu_ != nullptr);
	m.addSeparator();
	m.addItem(++item_index, "Save state", cpu_ != nullptr);
	m.addItem(++item_index, "Load state", cpu_ != nullptr);
	m.addSeparator();
	m.addItem(++item_index, "Enable CPU debugging", cpu_debug_component_ != nullptr, cpu_debug_component_ && cpu_debug_component_->isVisible());
	m.addItem(++item_index, "Enable memory map", memory_map_component_ != nullptr, memory_map_component_ && memory_map_component_->isVisible());
	m.addItem(++item_index, "Enable graphics debugging", ppu_debug_component_ != nullptr, ppu_debug_component_ && ppu_debug_component_->isVisible());
	const int result = m.show();

	switch (result)
	{
	case 0:
		// Did not select anything
		break;
	case 1:
		{FileChooser rom_chooser{ "Select a ROM file to load...", File::getSpecialLocation(File::currentExecutableFile), "*.gb" };
		if (rom_chooser.browseForFileToOpen()) {
			auto rom_file = rom_chooser.getResult();
			try
			{
				LoadRom(rom_file.getFullPathName().toStdString());
				StartEmulation();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", String{ "Error: " } +e.what());
			}
		}}
		break;
	case 2:
		try
		{
			LoadRom(loaded_rom_file_path_);
			StartEmulation();
		}
		catch (std::exception &e)
		{
			AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", String{ "Error: " } +e.what());
		}
		break;
	case 3:
		SaveState();
		break;
	case 4:
		LoadState();
		break;
	case 5:
		EnableDebugging(*cpu_debug_component_, !cpu_debug_component_->isVisible());
		break;
	case 6:
		EnableDebugging(*memory_map_component_, !memory_map_component_->isVisible());
		break;
	case 7:
		EnableDebugging(*ppu_debug_component_, !ppu_debug_component_->isVisible());
		break;
	default:
		break;
	}

	if (was_cpu_running && cpu_) { StartEmulation(); }
}

bool JucyBoy::keyPressed(const KeyPress &key)
{
	// Switch statement does not work below because the keys are not compile time constants...
	if (key.getKeyCode() == KeyPress::spaceKey)
	{
		if (!cpu_) { return true; }
		if (cpu_->IsRunning())
		{
			PauseEmulation();
			UpdateDebugComponents(true);
		}
		else
		{
			StartEmulation();
		}
	}
	else if (key.getKeyCode() == KeyPress::rightKey)
	{
		if (!cpu_) { return true; }
		if (!cpu_->IsRunning())
		{
			try
			{
				cpu_debug_component_->isVisible() ? cpu_->DebugStepOver() : cpu_->StepOver();
			}
			catch (std::exception &e)
			{
				AlertWindow::showMessageBox(AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
			}
			UpdateDebugComponents(true);
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

	if (joypad_) joypad_->UpdatePressedKeys(pressed_keys);

	return true;
}

// The listener callback is called from within the CPU's running loop.
// The call has to be forwarded to the message thread in order to join the running loop thread.
// Moreover, any update to the GUI components (as the listener callback of Reset) can only be done safely in the message thread.
void JucyBoy::OnRunningLoopInterrupted()
{
	MessageManager::callAsync([this]() {
		PauseEmulation();
		UpdateDebugComponents(true);
	});
}

void JucyBoy::EnableDebugging(Component &component, bool enable)
{
	component.setVisible(enable);
	setSize(ComputeWindowWidth(), 144 * 4);
}

int JucyBoy::ComputeWindowWidth() const
{
	int total_width{ 160 * 4 };
	if (cpu_debug_component_ && cpu_debug_component_->isVisible()) total_width += cpu_status_width_;
	if (memory_map_component_ && memory_map_component_->isVisible()) total_width += memory_map_width_;
	if (ppu_debug_component_ && ppu_debug_component_->isVisible()) total_width += ppu_tileset_width_;
	return total_width;
}
