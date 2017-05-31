#pragma once

#include <array>
#include <cstdint>
#include "Sprite.h"
#include "CPU.h"

class MMU;

class PPU : public CPU::Listener
{
public:
	enum class State
	{
		HBLANK = 0,
		VBLANK = 1,
		OAM = 2,
		VRAM = 3
	};

	enum class Color : uint8_t
	{
		White = 0,
		LightGrey = 1,
		DarkGrey = 2,
		Black = 3
	};

	using Palette = std::array<Color, 4>;
	using Tile = std::array<uint8_t, 8 * 8>;
	using Framebuffer = std::array<Color, 160 * 144>;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnNewFrame(const Framebuffer &/*framebuffer*/) {}
	};

public:
	PPU(MMU &mmu);
	virtual ~PPU();

	// CPU::Listener overrides
	void OnMachineCycleLapse() override;

	// MMU listener functions
	void OnVramWritten(Memory::Address address, uint8_t value);
	void OnOamWritten(Memory::Address address, uint8_t value);
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	// Rendering
	void RenderBackground(uint8_t line_number);
	void RenderWindow(uint8_t line_number);
	void RenderSprites(uint8_t line_number);

	// Register write functions
	void SetLcdControl(uint8_t value);
	void SetLcdStatus(uint8_t value);
	void SetPaletteData(Palette &palette, uint8_t value);

	// Helper functions
	void EnableLcd(bool enabled);
	void SetLcdState(State state);
	uint8_t IncrementLine() { return SetLineNumber(current_line_ + 1); }
	uint8_t SetLineNumber(uint8_t line_number);
	void UpdateLineComparison();

	// Listener notification
	void NotifyNewFrame() const;

protected:
	// LCD mode state machine
	State current_state_{ State::VBLANK };
	CPU::MachineCycles cycles_lapsed_in_state_{ 0 };

	// LCD Control register values
	bool show_bg_{ true }; // bit 0
	bool show_sprites_{ false }; // bit 1
	bool double_size_sprites_{ false }; // bit 2
	size_t active_bg_tile_map_{ 0 }; // bit 3
	size_t active_tile_set_{ 1 }; // bit 4
	bool show_window_{ false }; // bit 5
	size_t active_window_tile_map_{ 0 }; // bit 6
	bool lcd_on_{ true }; // bit 7

	// LCD Status register values
	bool hblank_interrupt_enabled_{ false }; // bit 3
	bool vblank_interrupt_enabled_{ false }; // bit 4
	bool oam_interrupt_enabled_{ false }; // bit 5
	bool line_coincidence_interrupt_enabled_{ false }; // bit 6

	// Other register values
	uint8_t scroll_y_{ 0 };
	uint8_t scroll_x_{ 0 };
	uint8_t current_line_{ 0 };
	uint8_t line_compare_{ 0 };
	Palette bg_palette_{};
	std::array<Palette, 2> obj_palettes_{ { {}, {} } };
	int window_y_{ 0 };
	int window_x_{ 0 };

	std::array<Tile, 384> tile_set_{};

	using TileMap = std::array<uint8_t, 32 * 32>;
	std::array<TileMap, 2> tile_maps_{};

	std::array<Sprite, 40> sprites_{};

	std::array<bool, 160 * 144> is_bg_transparent_{}; // Color number 0 on background is "transparent" and therefore sprites show on top of it
	Framebuffer framebuffer_{};

	MMU* mmu_{ nullptr };
	std::set<Listener*> listeners_;

private:
	PPU(const PPU&) = delete;
	PPU(PPU&&) = delete;
	PPU& operator=(const PPU&) = delete;
	PPU& operator=(PPU&&) = delete;
};
