#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

class GameScreenComponent;

class GraphicOptionsComponent : public juce::Component, public juce::Button::Listener
{
public:
	GraphicOptionsComponent(GameScreenComponent &game_screen_component);
	~GraphicOptionsComponent() = default;

	void buttonClicked(juce::Button* button) override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	juce::ToggleButton nearest_mag_filter_toggle_;
	juce::ToggleButton linear_mag_filter_toggle_;

	GameScreenComponent* game_screen_component_;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphicOptionsComponent)
};
