#pragma once

#include "../PPU.h"

class DebugPPU final : public PPU
{
public:
	DebugPPU(MMU &mmu) : PPU(mmu) {}
	~DebugPPU() = default;

	std::array<Tile, 384> GetTileSet() const { return tile_set_; }
	//std::array<TileMap, 2> GetTileMaps() const { return tile_maps_; }
	//std::array<Sprite, 40> GetSprites() const { return sprites_; }
	Palette GetBgPalette() const { return bg_palette_; }

private:


private:
	DebugPPU(const DebugPPU&) = delete;
	DebugPPU(DebugPPU&&) = delete;
	DebugPPU& operator=(const DebugPPU&) = delete;
	DebugPPU& operator=(DebugPPU&&) = delete;
};
