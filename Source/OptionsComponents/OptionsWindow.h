#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GraphicOptionsComponent.h"
#include "AudioOptionsComponent.h"

class OptionsWindow : public juce::DocumentWindow
{
public:
	OptionsWindow(GameScreenComponent &game_screen_component, AudioPlayerComponent &audio_player_component, juce::LookAndFeel &look_and_feel) :
		juce::DocumentWindow{ "SuperJucyBoy Options", juce::Colours::white, DocumentWindow::closeButton },
		graphic_options_{ game_screen_component },
		audio_options_{ audio_player_component }
	{
		setLookAndFeel(&look_and_feel);
		tabbed_component_.setLookAndFeel(&look_and_feel);
		graphic_options_.setLookAndFeel(&look_and_feel);
		audio_options_.setLookAndFeel(&look_and_feel);

		tabbed_component_.addTab("Graphics", juce::Colours::white, &graphic_options_, true);
		tabbed_component_.addTab("Audio", juce::Colours::white, &audio_options_, true);

		setContentOwned(&tabbed_component_, false);
	}
	~OptionsWindow() = default;

	void closeButtonPressed() override
	{
		setVisible(false);
	}

private:
	juce::TabbedComponent tabbed_component_{ juce::TabbedButtonBar::TabsAtTop };
	GraphicOptionsComponent graphic_options_;
	AudioOptionsComponent audio_options_;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsWindow)
};
