#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <vector>
#include <memory>
#include "GameScreenComponent.h"
#include "AudioPlayerComponent.h"
#include "JucyBoy/CPU.h"
#include "OptionsComponents/OptionsComponent.h"
#include "DebugComponents/DebuggerComponent.h"
#include "AdditionalWindow.h"

class JucyBoy;

class JucyBoyComponent final : public juce::Component, public juce::ApplicationCommandTarget
{
public:
	JucyBoyComponent();
	~JucyBoyComponent();

	void paint(juce::Graphics&) override {}
	void resized() override;

	void mouseDown(const juce::MouseEvent &event) override;
	bool keyPressed(const juce::KeyPress &key) override;
	bool keyStateChanged(bool isKeyDown) override;

	// CPU::Listener overrides
	void OnRunningLoopInterrupted();

private:
	void LoadRom(std::string file_path);
	void StartEmulation();
	void PauseEmulation();

	// Save/load state
	void SaveState() const;
	void LoadState();
	void SelectSaveSlot(size_t selected_save_slot) { selected_save_slot_ = selected_save_slot; }

	// juce::ApplicationCommandTarget overrides
	ApplicationCommandTarget* getNextCommandTarget() override;
	void getAllCommands(juce::Array<juce::CommandID>& commands) override;
	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
	bool perform(const InvocationInfo& info) override;

private:
	std::unique_ptr<JucyBoy> jucy_boy_;

	std::vector<std::function<void()>> listener_deregister_functions_;

	std::string loaded_rom_file_path_;

	juce::LookAndFeel_V4 look_and_feel_{ juce::LookAndFeel_V4::getLightColourScheme() };

	GameScreenComponent game_screen_component_;
	AudioPlayerComponent audio_player_component_;

	OptionsComponent options_component_{ game_screen_component_, audio_player_component_ };
	AdditionalWindow options_window_{ options_component_, "JucyBoy Options", juce::Colours::white, juce::DocumentWindow::closeButton };
	DebuggerComponent debugger_component_;
	AdditionalWindow debugger_window_{ debugger_component_, "JucyBoy Debugger", juce::Colours::white, juce::DocumentWindow::closeButton };

	size_t selected_save_slot_{ 1 }; // Slot 0 is not used

	juce::ApplicationCommandManager application_command_manager_;
	// The command IDs have to be consecutive, due to the method used to feed them to the manager in getAllCommands
	enum CommandIDs
	{
		LoadRomFileCmd = 0x2000,
		ResetCmd,
		SaveStateCmd,
		LoadStateCmd,
		SelectSaveSlot1Cmd,
		SelectSaveSlot2Cmd,
		SelectSaveSlot3Cmd,
		SelectSaveSlot4Cmd,
		SelectSaveSlot5Cmd,
		SelectSaveSlot6Cmd,
		SelectSaveSlot7Cmd,
		SelectSaveSlot8Cmd,
		EnableDebuggingCmd,
		ViewOptionsCmd,
		CommandCount // Always leave this last
	};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoyComponent)
};
