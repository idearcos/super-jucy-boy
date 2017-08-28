#include "JucyBoyComponent.h"
#include "JucyBoy/JucyBoy.h"
#include <fstream>
#include <cassert>
#include "cereal/archives/binary.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"

JucyBoyComponent::JucyBoyComponent()
{
	setLookAndFeel(&look_and_feel_);
	setSize(160 * 4, 144 * 4);
	setWantsKeyboardFocus(true);

	game_screen_component_.addMouseListener(this, true);
	addAndMakeVisible(game_screen_component_);

	addChildComponent(audio_player_component_);

	// Options window
	options_window_.setLookAndFeel(&look_and_feel_);

	// Debugger window
	debugger_window_.setLookAndFeel(&look_and_feel_);

	application_command_manager_.registerAllCommandsForTarget(this);
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

		debugger_component_.SetJucyBoy(*jucy_boy_);

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

	jucy_boy_->StartEmulation(debugger_window_.isVisible());
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

	debugger_component_.OnEmulationPaused();
}

void JucyBoyComponent::paint(juce::Graphics& g)
{
}

void JucyBoyComponent::resized()
{
	auto working_area = getLocalBounds();
	game_screen_component_.setBounds(working_area.removeFromLeft(160 * 4).removeFromTop(144 * 4));
}

void JucyBoyComponent::SaveState() const
{
	if (loaded_rom_file_path_.empty()) return;

	auto save_state_file_path{ loaded_rom_file_path_ };
	const auto extension = ".jb" + std::to_string(selected_save_slot_);

	const auto last_dot_position = save_state_file_path.find_last_of('.');
	if (std::string::npos != last_dot_position)
	{
		save_state_file_path.replace(last_dot_position, std::string::npos, extension);
	}
	else
	{
		save_state_file_path.append(extension);
	}

	std::ofstream save_state_file{ save_state_file_path, std::ios::binary };

	{cereal::BinaryOutputArchive  output_archive{ save_state_file };
	output_archive(*jucy_boy_); }

	save_state_file.close();
}

void JucyBoyComponent::LoadState()
{
	if (loaded_rom_file_path_.empty()) return;

	auto save_state_file_path{ loaded_rom_file_path_ };
	const auto extension = ".jb" + std::to_string(selected_save_slot_);

	const auto last_dot_position = save_state_file_path.find_last_of('.');
	if (std::string::npos != last_dot_position)
	{
		save_state_file_path.replace(last_dot_position, std::string::npos, extension);
	}
	else
	{
		save_state_file_path.append(extension);
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
		debugger_component_.UpdateState(false);
	}

	juce::PopupMenu menu;
	menu.setLookAndFeel(&look_and_feel_);
	int item_index{ 0 };
	menu.addItem(++item_index, "Load ROM");
	menu.addItem(++item_index, "Reset", jucy_boy_ != nullptr);
	menu.addSeparator();
	menu.addCommandItem(&application_command_manager_, CommandIDs::SaveStateCmd);
	menu.addCommandItem(&application_command_manager_, CommandIDs::LoadStateCmd);
	menu.addSeparator();
	menu.addItem(++item_index, "Enable debugging", true, debugger_window_.isVisible());
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
				if (!debugger_window_.isVisible()) StartEmulation();
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
			if (!debugger_window_.isVisible()) StartEmulation();
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
		debugger_window_.setVisible(true);
		break;
	case 6:
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
			debugger_component_.UpdateState(true);
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
			jucy_boy_->StepOver(debugger_window_.isVisible());
		}
		catch (std::exception &e)
		{
			juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Exception caught in CPU: ", e.what());
		}
		debugger_component_.UpdateState(true);
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
		debugger_component_.UpdateState(true);
		game_screen_component_.UpdateFramebuffer();
	});
}

juce::ApplicationCommandTarget* JucyBoyComponent::getNextCommandTarget()
{
	return findFirstTargetParentComponent();
}

void JucyBoyComponent::getAllCommands(juce::Array<juce::CommandID>& commands)
{
	const juce::CommandID ids[] = { CommandIDs::SaveStateCmd, CommandIDs::LoadStateCmd };
	commands.addArray(ids, juce::numElementsInArray(ids));
}

void JucyBoyComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
	switch (commandID)
	{
	case CommandIDs::SaveStateCmd:
		result.setInfo("Save state", "Save state in selected slot", "Save states", 0);
		result.setActive(static_cast<bool>(jucy_boy_));
		result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
		break;
	case CommandIDs::LoadStateCmd:
		result.setInfo("Load state", "Load state from selected slot", "Save states", 0);
		result.setActive(static_cast<bool>(jucy_boy_));
		result.addDefaultKeypress('l', juce::ModifierKeys::commandModifier);
		break;
	default:
		break;
	}
}

bool JucyBoyComponent::perform(const InvocationInfo& info)
{
	switch (info.commandID)
	{
	case CommandIDs::SaveStateCmd:
		SaveState();
		break;
	case CommandIDs::LoadStateCmd:
		LoadState();
		break;
	default:
		return false;
	}

	return true;
}
