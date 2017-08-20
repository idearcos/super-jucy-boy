#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "GraphicOptionsComponent.h"
#include "AudioOptionsComponent.h"

class OptionsComponent : public juce::Component
{
public:
	OptionsComponent(GameScreenComponent &game_screen_component, AudioPlayerComponent &audio_player_component) :
		graphic_options_{ game_screen_component },
		audio_options_{ audio_player_component }
	{
		addAndMakeVisible(tabbed_component_);

		tabbed_component_.addTab("Graphics", juce::Colours::white, &graphic_options_, true);
		tabbed_component_.addTab("Audio", juce::Colours::white, &audio_options_, true);

		setSize(300, 100);
	}
	~OptionsComponent() = default;

	void paint(juce::Graphics&) override {}
	void resized() override
	{
		tabbed_component_.setBounds(getLocalBounds());
	}

private:
	juce::TabbedComponent tabbed_component_{ juce::TabbedButtonBar::TabsAtTop };
	GraphicOptionsComponent graphic_options_;
	AudioOptionsComponent audio_options_;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsComponent)
};
