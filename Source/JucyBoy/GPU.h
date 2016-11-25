#pragma once

#include <array>
#include <cstdint>
#include "CPU.h"
#include "MMU.h"

class GPU final : public CPU::Listener
{
public:
	GPU(MMU &mmu);
	~GPU();

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

	using Framebuffer = std::array<Color, 160 * 144>;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnNewFrame(const Framebuffer &/*framebuffer*/) {}
	};

	State GetCurrentState() { return current_state_; }

	// CPU::Listener overrides
	void OnCyclesLapsed(CPU::MachineCycles cycles) override;

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

	uint8_t IncrementLine() { return SetLineNumber(current_line_ + 1); }
	uint8_t SetLineNumber(uint8_t line_number);
	void UpdateLineComparison();

	// Register write callback functions
	void SetLcdControl(uint8_t value);
	void SetLcdStatus(uint8_t value);
	void SetPaletteData(uint8_t value);

	// Helper functions
	void EnableLcd(bool enabled);
	void SetLcdState(State state);

	// Listener notification
	void NotifyNewFrame() const;

private:
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
	std::array<Color, 4> palette_{};
	uint8_t window_y_{ 0 };
	uint8_t window_x_{ 0 };

	using Tile = std::array<uint8_t, 8 * 8>;
	std::array<Tile, 384> tile_set_{};

	using TileMap = std::array<uint8_t, 32 * 32>;
	std::array<TileMap, 2> tile_maps_{};

	std::array<uint8_t, 160 * 144> color_numbers_buffer_{}; // Color numbers before mapping to palette colors
	Framebuffer framebuffer_{};

	MMU* mmu_{ nullptr };
	std::set<Listener*> listeners_;

private:
	GPU(const GPU&) = delete;
	GPU(GPU&&) = delete;
	GPU& operator=(const GPU&) = delete;
	GPU& operator=(GPU&&) = delete;
};
