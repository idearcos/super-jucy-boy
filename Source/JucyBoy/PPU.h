#pragma once

#include <array>
#include <cstdint>
#include <list>
#include "Sprite.h"
#include "CPU.h"

class MMU;

class PPU
{
public:
	enum class State
	{
		// Apart from the LCD modes 0-3, some "virtual" modes are used to represent more specific situations of the LCD.
		// The lower 2 bits of these virtual modes represent the mode reported when reading the STAT register in these situations.
		// Therefore, a 0x03 mask is applied when the STAT register is read.

		HBLANK = 0,
		LcdTurnedOn = 4, // Used upon LCD being turned on, for line #0 (there is no OAM mode for this line before VRAM mode)

		VBLANK = 1,
		EnteredVBLANK = 5,
		VBLANK_Line153 = 9, // Line 153 of VBlank reports LY=153 for only 8 clock cycles...
		EnteredVBLANK_Line153 = 13, // VBLANK mode of line 153 requests LY=LYC for line 153 upon entering
		VBLANK_Line0 = 17, // The remaining clock cycles of VBLANK in line 153 report LY=0, and requests 
		EnteredVBLANK_Line0 = 21, // VBLANK mode of line 0 requests LY=LYC for line 0 upon entering

		OAM = 2,
		EnteredOAM = 6, // Used to trigger LY = LYC comparison at the beginning of OAM (4 clock cycles after LY is incremented)
		OAM_Line0 = 10, // Used for OAM mode of line 0

		VRAM = 3,
	};

	enum class Color : uint8_t
	{
		White = 0,
		LightGrey = 1,
		DarkGrey = 2,
		Black = 3,
		Count
	};

	using Framebuffer = std::array<Color, 160 * 144>;
	using Tile = std::array<uint8_t, 8 * 8>;
	using Palette = std::array<Color, 4>;

public:
	PPU(MMU &mmu);
	virtual ~PPU() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse();

	// MMU mapped memory read/write functions
	uint8_t OnVramRead(Memory::Address address) const;
	void OnVramWritten(Memory::Address address, uint8_t value);
	uint8_t OnOamRead(Memory::Address address) const;
	void OnOamWritten(Memory::Address address, uint8_t value);
	uint8_t OnIoMemoryRead(Memory::Address address);
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	// Listeners management
	using Listener = std::function<void()>;
	std::function<void()> AddNewFrameListener(Listener &&listener);

	// GUI interaction
	const Framebuffer& GetFramebuffer() const { return framebuffer_; }
	const std::array<Tile, 384>& GetTileSet() const { return tile_set_; }

	template<class Archive>
	void serialize(Archive &archive);

private:
	// Rendering
	void RenderBackground(uint8_t line_number, uint8_t x);
	void RenderWindow(uint8_t line_number, uint8_t x);
	std::vector<size_t> ComputeSpritesToRender(uint8_t line_number) const;
	size_t ComputeVramModeDuration() const;
	void RenderSprites(uint8_t line_number);

	// Register write functions
	void SetLcdControl(uint8_t value);
	void SetLcdStatus(uint8_t value);
	void SetPaletteData(Palette &palette, uint8_t value);

	// Helper functions
	void EnableLcd(bool enabled);
	uint8_t GetPaletteData(const Palette &palette) const;
	void WriteOam(size_t index, uint8_t value);
	inline bool CompareCurrentLine() const { return (current_line_ == line_compare_) && ((clock_cycles_lapsed_in_line_ >= 4) || (current_line_ == 0)); }

	// Listener notification
	void NotifyNewFrame() const;

private:
	static constexpr size_t oam_state_duration_{ 80 };
	static constexpr size_t vram_state_duration_{ 172 };
	static constexpr size_t hblank_state_duration_{ 204 };
	static constexpr size_t line_duration_{ 456 };

	static constexpr size_t vblank_line153_duration{ 8 };
	static constexpr size_t vblank_line0_duration{ line_duration_ - vblank_line153_duration };

	static constexpr uint16_t tile_map_0_offset_{ 0x1800 };
	static constexpr uint16_t tile_map_1_offset_{ 0x1C00 };

	// LCD mode state machine
	State current_state_{ State::LcdTurnedOn };
	State next_state_{ State::LcdTurnedOn };
	size_t clock_cycles_lapsed_in_state_{ 0 };
	size_t clock_cycles_lapsed_in_line_{ 0 };
	size_t hblank_duration_this_line_{ hblank_state_duration_ };
	size_t vram_duration_this_line_{ vram_state_duration_ };
	size_t scroll_x_delay_this_line_{ 0 };
	uint8_t x_to_render_{ 0 };

	// LCD Control register values
	bool show_bg_{ true };					// bit 0
	bool show_sprites_{ false };			// bit 1
	int sprite_height_{ 8 };				// bit 2
	size_t active_bg_tile_map_{ 0 };		// bit 3
	size_t active_tile_set_{ 1 };			// bit 4
	bool show_window_{ false };				// bit 5
	size_t active_window_tile_map_{ 0 };	// bit 6
	bool lcd_on_{ true };					// bit 7

	// LCD Status register values
	bool hblank_interrupt_enabled_{ false };			// bit 3
	bool vblank_interrupt_enabled_{ false };			// bit 4
	bool oam_interrupt_enabled_{ false };				// bit 5
	bool line_coincidence_interrupt_enabled_{ false };	// bit 6

	// Other register values
	uint8_t scroll_y_{ 0 };
	uint8_t scroll_x_{ 0 };
	uint8_t current_line_{ 0 };
	uint8_t line_compare_{ 0 };
	Palette bg_palette_;
	std::array<Palette, 2> obj_palettes_;
	int window_y_{ 0 };
	int window_x_{ -7 };

	std::array<uint8_t, Memory::vram_size_> vram_{};
	std::array<uint8_t, Memory::oam_size_> oam_{};
	std::array<Tile, 384> tile_set_{};

	using TileMap = std::array<uint8_t, 32 * 32>;
	std::array<TileMap, 2> tile_maps_{};

	std::array<Sprite, 40> sprites_{};
	std::vector<size_t> sprites_to_render_this_line_;

	Framebuffer framebuffer_{};
	std::array<bool, 160 * 144> is_bg_transparent_{}; // Color number 0 on background is "transparent" and therefore sprites show on top of it

	struct OamDma
	{
		enum class State
		{
			Startup, // 1 cycle
			Active, // 160 cycles
			Teardown, // 1 cycle
			Inactive
		};
		State current_state_{ State::Inactive };
		State next_state_{ State::Inactive };
		Memory::Address source_{ 0x0000 };
		uint8_t current_byte_index_{ 0 };
	} oam_dma_;

	MMU* mmu_{ nullptr };
	std::list<Listener> listeners_;

private:
	PPU(const PPU&) = delete;
	PPU(PPU&&) = delete;
	PPU& operator=(const PPU&) = delete;
	PPU& operator=(PPU&&) = delete;
};

template<class Archive>
void PPU::serialize(Archive & archive)
{
	archive(current_state_, next_state_);
	archive(clock_cycles_lapsed_in_state_, clock_cycles_lapsed_in_line_, vram_duration_this_line_, hblank_duration_this_line_);
	archive(show_bg_, show_sprites_, sprite_height_, active_bg_tile_map_, active_tile_set_, show_window_, active_window_tile_map_, lcd_on_);
	archive(hblank_interrupt_enabled_, vblank_interrupt_enabled_, oam_interrupt_enabled_, line_coincidence_interrupt_enabled_);
	archive(scroll_y_, scroll_x_, current_line_, line_compare_);
	archive(bg_palette_, obj_palettes_);
	archive(window_y_, window_x_);
	archive(vram_, oam_);
	archive(tile_set_, tile_maps_);
	archive(sprites_, sprites_to_render_this_line_);
	archive(framebuffer_, is_bg_transparent_);
	archive(oam_dma_.current_state_, oam_dma_.next_state_, oam_dma_.source_, oam_dma_.current_byte_index_);
}
