#include "JucyBoy.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include "cereal/archives/binary.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

JucyBoy::JucyBoy()
{
	setLookAndFeel(&look_and_feel_);

	setSize(ComputeWindowWidth(), 144 * 4);
	setWantsKeyboardFocus(true);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	addChildComponent(audio_player_component_);

	// Debug components
	cpu_debug_component_.addMouseListener(this, true);
	addChildComponent(cpu_debug_component_);
	EnableDebugging(cpu_debug_component_, false);

	memory_map_component_.addMouseListener(this, true);
	addChildComponent(memory_map_component_);
	EnableDebugging(memory_map_component_, false);

	ppu_debug_component_.addMouseListener(this, true);
	addChildComponent(ppu_debug_component_);
	EnableDebugging(ppu_debug_component_, false);
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
	timer_ = std::make_unique<Timer>(*mmu_);
	joypad_ = std::make_unique<Joypad>();
	cartridge_ = std::make_unique<Cartridge>(file_path);

	// Register all listener functions
	// CPU listeners
	cpu_->CPU::AddListener(*timer_);
	cpu_->CPU::AddListener(*ppu_);
	cpu_->CPU::AddListener(*apu_);
	cpu_->CPU::AddListener(*this);

	// Map memory read/write functions to MMU
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return cartridge_->OnRomBank0Read(address); }, Memory::Region::ROM_Bank0);
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return cartridge_->OnRomBankNRead(address); }, Memory::Region::ROM_OtherBanks);
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return cartridge_->OnExternalRamRead(address); }, Memory::Region::ERAM);
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return ppu_->OnVramRead(address); }, Memory::Region::VRAM);
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return ppu_->OnOamRead(address); }, Memory::Region::OAM);
	mmu_->MapMemoryRead([this](const Memory::Address &address) { return cpu_->OnInterruptsRead(address); }, Memory::Region::Interrupts);

	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { cartridge_->OnRomBank0Written(address, value); }, Memory::Region::ROM_Bank0);
	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { cartridge_->OnRomBankNWritten(address, value); }, Memory::Region::ROM_OtherBanks);
	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { cartridge_->OnExternalRamWritten(address, value); }, Memory::Region::ERAM);
	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { ppu_->OnVramWritten(address, value); }, Memory::Region::VRAM);
	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { ppu_->OnOamWritten(address, value); }, Memory::Region::OAM);
	mmu_->MapMemoryWrite([this](const Memory::Address &address, uint8_t value) { cpu_->OnInterruptsWritten(address, value); }, Memory::Region::Interrupts);

	// Map IO register read/write functions to MMU
	mmu_->MapIoRegisterRead([this](const Memory::Address &address) { return cpu_->OnIoMemoryRead(address); }, Memory::IF, Memory::IF);
	mmu_->MapIoRegisterRead([this](const Memory::Address &address) { return ppu_->OnIoMemoryRead(address); }, Memory::LCDC, Memory::WX);
	mmu_->MapIoRegisterRead([this](const Memory::Address &address) { return apu_->OnIoMemoryRead(address); }, Memory::NR10, Memory::WaveEnd);
	mmu_->MapIoRegisterRead([this](const Memory::Address &address) { return timer_->OnIoMemoryRead(address); }, Memory::DIV, Memory::TAC);
	mmu_->MapIoRegisterRead([this](const Memory::Address &address) { return joypad_->OnIoMemoryRead(address); }, Memory::JOYP, Memory::JOYP);

	mmu_->MapIoRegisterWrite([this](const Memory::Address &address, uint8_t value) { cpu_->OnIoMemoryWritten(address, value); }, Memory::IF, Memory::IF);
	mmu_->MapIoRegisterWrite([this](const Memory::Address &address, uint8_t value) { ppu_->OnIoMemoryWritten(address, value); }, Memory::LCDC, Memory::WX);
	mmu_->MapIoRegisterWrite([this](const Memory::Address &address, uint8_t value) { apu_->OnIoMemoryWritten(address, value); }, Memory::NR10, Memory::WaveEnd);
	mmu_->MapIoRegisterWrite([this](const Memory::Address &address, uint8_t value) { timer_->OnIoMemoryWritten(address, value); }, Memory::DIV, Memory::TAC);
	mmu_->MapIoRegisterWrite([this](const Memory::Address &address, uint8_t value) { joypad_->OnIoMemoryWritten(address, value); }, Memory::JOYP, Memory::JOYP);

	// PPU listeners
	ppu_->AddListener(game_screen_component_);

	// APU listeners
	listener_deregister_functions_.emplace_back(apu_->AddListener([this](APU::SampleBatch &sample_batch) { audio_player_component_.OnNewSamples(sample_batch); }));

	loaded_rom_file_path_ = std::move(file_path);

	cpu_debug_component_.SetCpu(*cpu_);
	memory_map_component_.SetMmu(*mmu_);
	ppu_debug_component_.SetPpu(*ppu_);
}

void JucyBoy::StartEmulation()
{
	if (!cpu_ || cpu_->IsRunning()) return;

	cpu_debug_component_.isVisible() ? cpu_->DebugRun() : cpu_->Run();

	cpu_debug_component_.OnEmulationStarted();
	memory_map_component_.OnEmulationStarted();
}

void JucyBoy::PauseEmulation()
{
	if (!cpu_ || !cpu_->IsRunning()) return;

	try
	{
		// Join the thread. If an exception was thrown in the running loop, Stop will rethrow it.
		cpu_->Stop();
	}
	catch (std::exception &e)
	{
		juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
	}

	cpu_debug_component_.OnEmulationPaused();
	memory_map_component_.OnEmulationPaused();
}

void JucyBoy::UpdateDebugComponents(bool compute_diff)
{
	cpu_debug_component_.UpdateState(compute_diff);
	memory_map_component_.UpdateState(compute_diff);
	ppu_debug_component_.UpdateState();
}

void JucyBoy::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.setFont(14.0f);

	std::stringstream usage_instructions;
	usage_instructions << "Space: run / stop" << std::endl;
	usage_instructions << "Right: step over" << std::endl;
	g.drawFittedText(usage_instructions.str(), usage_instructions_area_, juce::Justification::centred, 2);

	g.drawRect(usage_instructions_area_, 1);
}

void JucyBoy::resized()
{
	auto working_area = getLocalBounds();
	game_screen_component_.setBounds(working_area.removeFromLeft(160 * 4).removeFromTop(144 * 4));

	if (cpu_debug_component_.isVisible())
	{
		auto cpu_debug_area = working_area.removeFromLeft(cpu_status_width_);
		usage_instructions_area_ = cpu_debug_area.removeFromTop(40);
		cpu_debug_component_.setBounds(cpu_debug_area);
	}

	if (memory_map_component_.isVisible())
	{
		auto memory_debug_area = working_area.removeFromLeft(memory_map_width_);
		memory_map_component_.setBounds(memory_debug_area);
	}

	if (ppu_debug_component_.isVisible())
	{
		auto ppu_tileset_area = working_area.removeFromLeft(ppu_tileset_width_);
		ppu_debug_component_.setBounds(ppu_tileset_area);
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

void JucyBoy::mouseDown(const juce::MouseEvent &event)
{
	if (!event.mods.isRightButtonDown()) { return; }

	bool was_cpu_running{ cpu_ ? cpu_->IsRunning() : false };
	if (was_cpu_running)
	{
		PauseEmulation();
		UpdateDebugComponents(false);
	}

	juce::PopupMenu menu;
	menu.setLookAndFeel(&look_and_feel_);
	int item_index{ 0 };
	menu.addItem(++item_index, "Load ROM");
	menu.addItem(++item_index, "Reset", cpu_ != nullptr);
	menu.addSeparator();
	menu.addItem(++item_index, "Save state", cpu_ != nullptr);
	menu.addItem(++item_index, "Load state", cpu_ != nullptr);
	menu.addSeparator();
	menu.addItem(++item_index, "Enable CPU debugging", true, cpu_debug_component_.isVisible());
	menu.addItem(++item_index, "Enable memory map", true, memory_map_component_.isVisible());
	menu.addItem(++item_index, "Enable graphics debugging", true, ppu_debug_component_.isVisible());
	const int result = menu.show();

	switch (result)
	{
	case 0:
		// Did not select anything
		break;
	case 1:
		{juce::FileChooser rom_chooser{ "Select a ROM file to load...", juce::File::getSpecialLocation(juce::File::currentExecutableFile), "*.gb" };
		if (rom_chooser.browseForFileToOpen()) {
			auto rom_file = rom_chooser.getResult();
			try
			{
				LoadRom(rom_file.getFullPathName().toStdString());
				if (!cpu_debug_component_.isVisible()) StartEmulation();
			}
			catch (std::exception &e)
			{
				juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", juce::String{ "Error: " } + e.what());
			}
		}}
		break;
	case 2:
		try
		{
			LoadRom(loaded_rom_file_path_);
			if (!cpu_debug_component_.isVisible()) StartEmulation();
		}
		catch (std::exception &e)
		{
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to open ROM file", juce::String{ "Error: " } + e.what());
		}
		break;
	case 3:
		SaveState();
		break;
	case 4:
		LoadState();
		break;
	case 5:
		EnableDebugging(cpu_debug_component_, !cpu_debug_component_.isVisible());
		break;
	case 6:
		EnableDebugging(memory_map_component_, !memory_map_component_.isVisible());
		break;
	case 7:
		EnableDebugging(ppu_debug_component_, !ppu_debug_component_.isVisible());
		break;
	default:
		break;
	}

	if (was_cpu_running && cpu_) { StartEmulation(); }
}

bool JucyBoy::keyPressed(const juce::KeyPress &key)
{
	// Switch statement does not work below because the keys are not compile time constants...
	if (key.getKeyCode() == juce::KeyPress::spaceKey)
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
	else if (key.getKeyCode() == juce::KeyPress::rightKey)
	{
		if (!cpu_) { return true; }
		if (!cpu_->IsRunning())
		{
			try
			{
				cpu_debug_component_.isVisible() ? cpu_->DebugStepOver() : cpu_->StepOver();
			}
			catch (std::exception &e)
			{
				juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
			}
			UpdateDebugComponents(true);
		}
	}

	return true;
}

bool JucyBoy::keyStateChanged(bool /*isKeyDown*/)
{
	std::vector<Joypad::Keys> pressed_keys;

	if (juce::KeyPress::isKeyCurrentlyDown('a'))
	{
		pressed_keys.push_back(Joypad::Keys::Left);
	}
	if (juce::KeyPress::isKeyCurrentlyDown('s'))
	{
		pressed_keys.push_back(Joypad::Keys::Down);
	}
	if (juce::KeyPress::isKeyCurrentlyDown('d'))
	{
		pressed_keys.push_back(Joypad::Keys::Right);
	}
	if (juce::KeyPress::isKeyCurrentlyDown('w'))
	{
		pressed_keys.push_back(Joypad::Keys::Up);
	}
	if (juce::KeyPress::isKeyCurrentlyDown('j'))
	{
		pressed_keys.push_back(Joypad::Keys::B);
	}
	if (juce::KeyPress::isKeyCurrentlyDown('k'))
	{
		pressed_keys.push_back(Joypad::Keys::A);
	}
	if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::returnKey))
	{
		pressed_keys.push_back(Joypad::Keys::Start);
	}
	if (juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::tabKey))
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
	juce::MessageManager::callAsync([this]() {
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
	if (cpu_debug_component_.isVisible()) total_width += cpu_status_width_;
	if (memory_map_component_.isVisible()) total_width += memory_map_width_;
	if (ppu_debug_component_.isVisible()) total_width += ppu_tileset_width_;
	return total_width;
}
