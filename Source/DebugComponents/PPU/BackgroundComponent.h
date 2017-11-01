#pragma once

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "BackgroundRenderer.h"
#include "../../JucyBoy/PPU.h"

class BackgroundComponent : public juce::Component, public juce::ButtonListener
{
public:
	BackgroundComponent(BackgroundRenderer &background_renderer);
	~BackgroundComponent();

	void SetPpu(PPU* ppu) { ppu_ = ppu; }

	// juce::ButtonListener overrides
	void buttonClicked(juce::Button* button);

	// juce::Component overrides
	void paint(juce::Graphics&) override;
	void resized() override;

private:
	juce::ToggleButton select_tile_map_auto_;
	juce::ToggleButton select_tile_map_0_;
	juce::ToggleButton select_tile_map_1_;

	PPU* ppu_{ nullptr };
	BackgroundRenderer* background_renderer_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackgroundComponent)
};
