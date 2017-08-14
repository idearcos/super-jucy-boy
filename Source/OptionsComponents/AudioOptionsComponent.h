#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

class AudioPlayerComponent;

class AudioOptionsComponent : public juce::Component, public juce::Button::Listener
{
public:
	AudioOptionsComponent(AudioPlayerComponent &audio_player_component);
	~AudioOptionsComponent() = default;

	void buttonClicked(juce::Button* button) override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	juce::ToggleButton toggle_audio_channel_1_;
	juce::ToggleButton toggle_audio_channel_2_;
	juce::ToggleButton toggle_audio_channel_3_;
	juce::ToggleButton toggle_audio_channel_4_;

	AudioPlayerComponent* audio_player_component_;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioOptionsComponent)
};
