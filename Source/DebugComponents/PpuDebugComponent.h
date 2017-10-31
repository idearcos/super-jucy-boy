#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PPU/TilesetComponent.h"
#include "PPU/BackgroundComponent.h"
#include "../JucyBoy/PPU.h"
#include <vector>
#include <functional>

class PpuDebugComponent final : public juce::OpenGLAppComponent, public PPU::Listener, public juce::ChangeListener
{
public:
	PpuDebugComponent();
	~PpuDebugComponent();

	void SetPpu(PPU* ppu);

	void Update();

	// juce::OpenGLAppComponent overrides
	void render();
	void initialise();
	void shutdown();

	// juce::Component overrides
	void paint(juce::Graphics&) override;
	void resized() override;
	void visibilityChanged() override;

	// juce::ChangeListener overrides
	void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
	juce::TabbedButtonBar tabbed_button_bar_{ juce::TabbedButtonBar::TabsAtTop };
	TilesetRenderer tileset_renderer_;
	BackgroundRenderer background_renderer_;
	juce::Component opengl_canvas_component_;

	PPU* ppu_{ nullptr };

	std::vector<std::function<void()>> listener_deregister_functions_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PpuDebugComponent)
};
