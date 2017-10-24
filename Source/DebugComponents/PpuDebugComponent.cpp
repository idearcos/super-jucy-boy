#include "PpuDebugComponent.h"

PpuDebugComponent::PpuDebugComponent()
{
	addAndMakeVisible(tabbed_component_);

	tabbed_component_.addTab("Tile Set", juce::Colours::white, &tileset_component_, true);
	tabbed_component_.addTab("Background", juce::Colours::white, &background_component_, true);

	setSize(BackgroundComponent::bg_width_in_tiles_ * BackgroundComponent::tile_width_ * 2, BackgroundComponent::bg_height_in_tiles_ * BackgroundComponent::tile_height_ * 2);
}

void PpuDebugComponent::SetPpu(PPU* ppu)
{
	// Clear previous listener interfaces
	for (auto &deregister : listener_deregister_functions_) { deregister(); }
	listener_deregister_functions_.clear();

	ppu_ = ppu;

	tileset_component_.SetPpu(ppu);
	background_component_.SetPpu(ppu);

	if (ppu != nullptr)
	{
		// Set listener interfaces
		listener_deregister_functions_.emplace_back(ppu->AddNewFrameListener([this]() { this->Update(); }));
	}
}

void PpuDebugComponent::Update()
{
	tileset_component_.UpdateTileSet();
	background_component_.Update();
}

void PpuDebugComponent::paint(juce::Graphics &g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);

	g.setFont(14.0f);
}

void PpuDebugComponent::resized()
{
	tabbed_component_.setBounds(getLocalBounds());
}

void PpuDebugComponent::visibilityChanged()
{
	// When closing the CPU debugger component, remove all interfaces with JucyBoy
	if (!isVisible())
	{
		SetPpu(nullptr);
	}
}
