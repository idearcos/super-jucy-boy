#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <array>

namespace Memory
{
	using Address = uint16_t;
	using Map = std::array<uint8_t, std::numeric_limits<Address>::max() + 1>;

	enum Region
	{
		ROM_Bank0,		// 0x0000 - 0x3FFF
		ROM_OtherBanks,	// 0x4000 - 0x7FFF
		VRAM,		// 0x8000 - 0x9FFF
		ERAM,		// 0xA000 - 0xBFFF
		WRAM,		// 0xC000 - 0xDFFF
		WRAM_Echo,	// 0xE000 - 0xFDFF
		OAM,		// 0xFE00 - 0xFE9F
		Unused,		// 0xFEA0 - 0xFEFF
		IO,			// 0xFF00 - 0xFF7F
		HRAM,		// 0xFF80 - 0xFFFE
		Interrupts	// 0xFFFF - 0xFFFF
	};

	std::pair<Region, Memory::Address> GetRegionAndRelativeAddress(Address address);

	static const size_t rom_bank_size_{ 0x4000 };
	static const size_t external_ram_bank_size_{ 0x2000 };

	static const std::vector<size_t> region_sizes_{ rom_bank_size_, rom_bank_size_, 0x2000, external_ram_bank_size_, 0x2000, 0x1E00, 0x00A0, 0x0060, 0x0080, 0x007F, 0x0001 };
	size_t inline GetSizeOfRegion(Region region) { return region_sizes_[static_cast<size_t>(region)]; }

	static const Address isr_start_{ 0x0040 };

	static const Address tile_sets_start_{ 0x8000 };
	static const Address tile_map_0_start_{ 0x9800 };
	static const Address tile_map_1_start_{ 0x9C00 };

	static const Address external_ram_start_{ 0xA000 };

	static const Address oam_start_{ 0xFE00 };

#pragma region IO addresses
	static const Address io_region_start_{ 0xFF00 };
	static const Address joypad_register_{ 0xFF00 };

	// Timer
	static const Address divider_register_{ 0xFF04 };				// DIV
	static const Address timer_counter_register_{ 0xFF05 };			// TIMA
	static const Address timer_modulo_register_{ 0xFF06 };			// TMA
	static const Address timer_control_register_{ 0xFF07 };			// TAC

	static const Address interrupt_flags_register_{ 0xFF0F };		// IF

	// GPU
	static const Address lcd_control_register_{ 0xFF40 };			// LCDC
	static const Address lcd_status_register_{ 0xFF41 };			// STAT
	static const Address scroll_y_register_{ 0xFF42 };				//SCY
	static const Address scroll_x_register_{ 0xFF43 };				// SCX
	static const Address current_line_register_{ 0xFF44 };			// LY
	static const Address line_compare_register_{ 0xFF45 };			// LYC
	static const Address dma_transfer_source_register_{ 0xFF46 };	// DMA
	static const Address bg_palette_register_{ 0xFF47 };			// BGP
	static const Address obj_palette_0_register_{ 0xFF48 };			// OBP0
	static const Address obj_palette_1_register_{ 0xFF49 };			// OBP1
	static const Address window_y_register_{ 0xFF4A };				// WY
	static const Address window_x_minus_seven_register_{ 0xFF4B };	// WX
#pragma endregion
}
