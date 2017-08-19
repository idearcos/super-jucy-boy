#include "JucyBoyComponent.h"
#include "JucyBoy/JucyBoy.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include "cereal/archives/binary.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

JucyBoyComponent::JucyBoyComponent()
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

	// Options window
	juce::Rectangle<int> area(0, 0, 300, 100);
	juce::RectanglePlacement placement(juce::RectanglePlacement::xRight | juce::RectanglePlacement::yTop | juce::RectanglePlacement::doNotResize);
	juce::Rectangle<int> result(placement.appliedTo(area, juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea.reduced(20)));
	options_window_.setBounds(result);
}

JucyBoyComponent::~JucyBoyComponent()
{
	PauseEmulation();
}

void JucyBoyComponent::LoadRom(std::string file_path)
{
	audio_player_component_.ClearBuffer();

	try
	{
		jucy_boy_ = std::make_unique<JucyBoy>(file_path);

		jucy_boy_->GetCpu().CPU::AddListener(*this);
		game_screen_component_.SetPpu(jucy_boy_->GetPpu());
		listener_deregister_functions_.emplace_back(jucy_boy_->GetApu().AddListener([this](APU::SampleBatch &sample_batch) { audio_player_component_.OnNewSamples(sample_batch); }));

		cpu_debug_component_.SetCpu(jucy_boy_->GetCpu());
		memory_map_component_.SetMmu(jucy_boy_->GetMmu());
		ppu_debug_component_.SetPpu(jucy_boy_->GetPpu());

		loaded_rom_file_path_ = std::move(file_path);
	}
	catch (std::exception &e)
	{
		juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Failed to load ROM: ", e.what());
	}
}

void JucyBoyComponent::StartEmulation()
{
	if (!jucy_boy_ || jucy_boy_->IsRunning()) return;

	jucy_boy_->StartEmulation(cpu_debug_component_.isVisible());

	cpu_debug_component_.OnEmulationStarted();
	memory_map_component_.OnEmulationStarted();
}

void JucyBoyComponent::PauseEmulation()
{
	if (!jucy_boy_ || !jucy_boy_->IsRunning()) return;

	try
	{
		// Join the thread. If an exception was thrown in the running loop, Stop will rethrow it.
		jucy_boy_->PauseEmulation();
	}
	catch (std::exception &e)
	{
		juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
	}

	cpu_debug_component_.OnEmulationPaused();
	memory_map_component_.OnEmulationPaused();
}

void JucyBoyComponent::UpdateDebugComponents(bool compute_diff)
{
	cpu_debug_component_.UpdateState(compute_diff);
	memory_map_component_.UpdateState(compute_diff);
	ppu_debug_component_.UpdateState();
}

void JucyBoyComponent::paint(juce::Graphics& g)
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

void JucyBoyComponent::resized()
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

void JucyBoyComponent::SaveState() const
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
	output_archive(*jucy_boy_); }

	save_state_file.close();
}

void JucyBoyComponent::LoadState()
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
	input_archive(*jucy_boy_); }

	save_state_file.close();
}

void JucyBoyComponent::mouseDown(const juce::MouseEvent &event)
{
	if (!event.mods.isRightButtonDown()) { return; }

	bool was_cpu_running{ jucy_boy_ ? jucy_boy_->IsRunning() : false };
	if (was_cpu_running)
	{
		PauseEmulation();
		UpdateDebugComponents(false);
	}

	juce::PopupMenu menu;
	menu.setLookAndFeel(&look_and_feel_);
	int item_index{ 0 };
	menu.addItem(++item_index, "Load ROM");
	menu.addItem(++item_index, "Reset", jucy_boy_ != nullptr);
	menu.addSeparator();
	menu.addItem(++item_index, "Save state", jucy_boy_ != nullptr);
	menu.addItem(++item_index, "Load state", jucy_boy_ != nullptr);
	menu.addSeparator();
	menu.addItem(++item_index, "Enable CPU debugging", true, cpu_debug_component_.isVisible());
	menu.addItem(++item_index, "Enable memory map", true, memory_map_component_.isVisible());
	menu.addItem(++item_index, "Enable graphics debugging", true, ppu_debug_component_.isVisible());
	menu.addSeparator();
	menu.addItem(++item_index, "Options...");
	const int selected_item = menu.show();

	switch (selected_item)
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
	case 8:
		options_window_.setVisible(true);
		break;
	default:
		break;
	}

	if (was_cpu_running && jucy_boy_) { StartEmulation(); }
}

bool JucyBoyComponent::keyPressed(const juce::KeyPress &key)
{
	// Switch statement does not work below because the keys are not compile time constants...
	if (key.getKeyCode() == juce::KeyPress::spaceKey)
	{
		if (!jucy_boy_) { return true; }
		if (jucy_boy_->IsRunning())
		{
			PauseEmulation();
			UpdateDebugComponents(true);
			game_screen_component_.UpdateFramebuffer();
		}
		else
		{
			StartEmulation();
		}
	}
	else if (key.getKeyCode() == juce::KeyPress::rightKey)
	{
		if (!jucy_boy_ || jucy_boy_->IsRunning()) { return true; }

		try
		{
			jucy_boy_->StepOver(cpu_debug_component_.isVisible());
		}
		catch (std::exception &e)
		{
			juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
		}
		UpdateDebugComponents(true);
		game_screen_component_.UpdateFramebuffer();
	}

	return true;
}

bool JucyBoyComponent::keyStateChanged(bool /*isKeyDown*/)
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

	if (jucy_boy_) jucy_boy_->GetJoypad().UpdatePressedKeys(pressed_keys);

	return true;
}

// The listener callback is called from within the CPU's running loop.
// The call has to be forwarded to the message thread in order to join the running loop thread.
// Moreover, any update to the GUI components (as the listener callback of Reset) can only be done safely in the message thread.
void JucyBoyComponent::OnRunningLoopInterrupted()
{
	juce::MessageManager::callAsync([this]() {
		PauseEmulation();
		UpdateDebugComponents(true);
		game_screen_component_.UpdateFramebuffer();
	});
}

void JucyBoyComponent::EnableDebugging(Component &component, bool enable)
{
	component.setVisible(enable);
	setSize(ComputeWindowWidth(), 144 * 4);
}

int JucyBoyComponent::ComputeWindowWidth() const
{
	int total_width{ 160 * 4 };
	if (cpu_debug_component_.isVisible()) total_width += cpu_status_width_;
	if (memory_map_component_.isVisible()) total_width += memory_map_width_;
	if (ppu_debug_component_.isVisible()) total_width += ppu_tileset_width_;
	return total_width;
}
