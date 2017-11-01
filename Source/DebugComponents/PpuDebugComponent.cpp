#include "PpuDebugComponent.h"

PpuDebugComponent::PpuDebugComponent()
{
	addAndMakeVisible(tabbed_button_bar_);
	tabbed_button_bar_.addChangeListener(this);
	tabbed_button_bar_.addTab("Tile Set", juce::Colours::white, 0);
	tabbed_button_bar_.addTab("Background", juce::Colours::white, 1);

	addAndMakeVisible(background_component_);
	addAndMakeVisible(opengl_canvas_component_);

	setSize(BackgroundRenderer::bg_width_in_tiles_ * BackgroundRenderer::tile_width_ * 2, BackgroundRenderer::bg_height_in_tiles_ * BackgroundRenderer::tile_height_ * 2);

	openGLContext.attachTo(opengl_canvas_component_);
	openGLContext.setComponentPaintingEnabled(false);
	openGLContext.setContinuousRepainting(false);
}

PpuDebugComponent::~PpuDebugComponent()
{
	shutdownOpenGL();
}

void PpuDebugComponent::SetPpu(PPU* ppu)
{
	// Clear previous listener interfaces
	for (auto &deregister : listener_deregister_functions_) { deregister(); }
	listener_deregister_functions_.clear();

	ppu_ = ppu;

	tileset_renderer_.SetPpu(ppu);
	background_renderer_.SetPpu(ppu);

	if (ppu != nullptr)
	{
		// Set listener interfaces
		listener_deregister_functions_.emplace_back(ppu->AddNewFrameListener([this]() { this->Update(); }));
	}
}

void PpuDebugComponent::Update()
{
	tileset_renderer_.Update();
	background_renderer_.Update();

	openGLContext.triggerRepaint();
}

void PpuDebugComponent::paint(juce::Graphics &g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void PpuDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	tabbed_button_bar_.setBounds(working_area.removeFromTop(30));

	if (background_component_.isVisible()) background_component_.setBounds(working_area.removeFromTop(30));

	opengl_canvas_component_.setBounds(working_area);
	tileset_renderer_.SetViewportArea(working_area);
	background_renderer_.SetViewportArea(working_area);
}

void PpuDebugComponent::visibilityChanged()
{
	// When closing the CPU debugger component, remove all interfaces with JucyBoy
	if (!isVisible())
	{
		SetPpu(nullptr);
	}
}

void PpuDebugComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
	background_component_.setVisible(tabbed_button_bar_.getCurrentTabIndex() == 1);

	resized();
}

void PpuDebugComponent::render()
{
	switch (tabbed_button_bar_.getCurrentTabIndex())
	{
	case 0:
		tileset_renderer_.render();
		break;
	case 1:
		background_renderer_.render();
		break;
	default:
		break;
	}
}

void PpuDebugComponent::initialise()
{
	tileset_renderer_.initialise();
	background_renderer_.initialise();
}

void PpuDebugComponent::shutdown()
{
	tileset_renderer_.shutdown();
}
