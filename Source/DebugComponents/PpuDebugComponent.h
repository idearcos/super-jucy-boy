#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PPU/TilesetComponent.h"
#include "PPU/BackgroundComponent.h"
#include "../JucyBoy/PPU.h"
#include <vector>
#include <functional>

class PpuDebugComponent final : public juce::Component, public PPU::Listener
{
public:
	PpuDebugComponent();
	~PpuDebugComponent() = default;

	void SetPpu(PPU* ppu);

	void Update();

	// juce::Component overrides
	void paint(juce::Graphics&) override;
	void resized() override;
	void visibilityChanged() override;

private:
	juce::TabbedComponent tabbed_component_{ juce::TabbedButtonBar::TabsAtTop };
	TilesetComponent tileset_component_;
	BackgroundComponent background_component_;

	PPU* ppu_{ nullptr };

	std::vector<std::function<void()>> listener_deregister_functions_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PpuDebugComponent)
};
