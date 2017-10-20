#include "PpuDebugComponent.h"

PpuDebugComponent::PpuDebugComponent()
{
	addAndMakeVisible(tileset_component_);
	addAndMakeVisible(background_component_);
}

void PpuDebugComponent::SetPpu(PPU* ppu)
{
	ppu_ = ppu;

	tileset_component_.SetPpu(ppu);
	background_component_.SetPpu(ppu);
}

void PpuDebugComponent::Update()
{
	tileset_component_.UpdateTileSet();
	background_component_.Update();
}

void PpuDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	tileset_component_.setBounds(working_area.removeFromLeft(TilesetComponent::tile_grid_width_ * TilesetComponent::tile_width_ * 2));
	background_component_.setBounds(working_area.removeFromLeft(BackgroundComponent::bg_width_in_tiles_ * BackgroundComponent::tile_width_ * 2));
}
