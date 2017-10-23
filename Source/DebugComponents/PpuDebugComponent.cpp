#include "PpuDebugComponent.h"

PpuDebugComponent::PpuDebugComponent()
{
	addAndMakeVisible(tileset_component_);
	addAndMakeVisible(background_component_);

	setSize(800, 600);
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

	g.drawFittedText("Tile set", tileset_label_area_, juce::Justification::centred, 2);
	g.drawRect(tileset_label_area_, 1);

	g.drawFittedText("Background", background_label_area_, juce::Justification::centred, 2);
	g.drawRect(background_label_area_, 1);
}

void PpuDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	auto tileset_area = working_area.removeFromLeft(TilesetComponent::tile_grid_width_ * TilesetComponent::tile_width_ * 2);
	tileset_label_area_ = tileset_area.removeFromTop(getHeight() - TilesetComponent::tile_grid_height_ * TilesetComponent::tile_height_ * 2);
	tileset_component_.setBounds(tileset_area);

	auto background_area = working_area.removeFromRight(BackgroundComponent::bg_width_in_tiles_ * BackgroundComponent::tile_width_ * 2);
	background_label_area_ = background_area.removeFromTop(getHeight() - BackgroundComponent::bg_height_in_tiles_ * BackgroundComponent::tile_height_ * 2);
	background_component_.setBounds(background_area);
}

void PpuDebugComponent::visibilityChanged()
{
	// When closing the CPU debugger component, remove all interfaces with JucyBoy
	if (!isVisible())
	{
		SetPpu(nullptr);
	}
}
