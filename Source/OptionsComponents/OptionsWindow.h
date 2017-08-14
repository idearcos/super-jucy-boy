#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioOptionsComponent.h"

class OptionsWindow : public juce::DocumentWindow
{
public:
	OptionsWindow(AudioPlayerComponent &audio_player_component, juce::LookAndFeel &look_and_feel) :
		juce::DocumentWindow{ "SuperJucyBoy Options", juce::Colours::white, DocumentWindow::closeButton },
		audio_options_{ audio_player_component }
	{
		setLookAndFeel(&look_and_feel);
		setContentOwned(&audio_options_, false);
	}
	~OptionsWindow() = default;

	void closeButtonPressed() override
	{
		setVisible(false);
	}

private:
	AudioOptionsComponent audio_options_;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsWindow)
};
