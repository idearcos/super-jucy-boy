#pragma once

#include "../OpenGl2DComponent.h"
#include "../JucyBoy/Debug/DebugPPU.h"

class PpuDebugComponent final : public OpenGl2DComponent
{
public:
	PpuDebugComponent(DebugPPU &debug_ppu);
	~PpuDebugComponent();

	// JucyBoy Listener functions
	void OnStatusUpdateRequested(bool compute_diff);

	// OpenGl2DComponent overrides
	void render() override;

	// juce::Component overrides
	void paint(Graphics&) override {}
	void resized() override {}

private:
	uint8_t PpuColorNumberToIntensity(uint8_t color_number);

private:
	static constexpr size_t num_tile_columns_{ 16 };
	static constexpr size_t num_tile_rows_{ 24 };
	static constexpr size_t tile_width_{ 8 };
	static constexpr size_t tile_height_{ 8 };
	static_assert(num_tile_columns_ * num_tile_rows_ == 384, "Invalid number of tile rows and columns");

	std::array<PPU::Tile, 384> tile_set_{};
	std::array<uint8_t, 384 * tile_width_ * tile_height_> framebuffer_;
	std::mutex framebuffer_mutex_;

	DebugPPU* debug_ppu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PpuDebugComponent)
};
