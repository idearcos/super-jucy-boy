#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <array>

namespace Memory
{
	using Address = uint16_t;
	using Map = std::array<uint8_t, std::numeric_limits<Address>::max() + 1>;

	enum class Region
	{
		ROM_Bank0,		// 0x0000 - 0x3FFF
		ROM_OtherBanks,	// 0x4000 - 0x7FFF
		VRAM,			// 0x8000 - 0x9FFF
		ERAM,			// 0xA000 - 0xBFFF
		WRAM,			// 0xC000 - 0xDFFF
		WRAM_Echo,		// 0xE000 - 0xFDFF
		OAM,			// 0xFE00 - 0xFE9F
		Unused,			// 0xFEA0 - 0xFEFF
		IO,				// 0xFF00 - 0xFF7F
		HRAM,			// 0xFF80 - 0xFFFE
		Interrupts		// 0xFFFF - 0xFFFF
	};

	struct Watchpoint
	{
		enum class Type
		{
			Read,
			Write
		};

		Watchpoint(Address address, Type type) : address(address), type(type) {}

		Address address{ 0x0000 };
		Type type{ Type::Write };
	};

	std::pair<Region, Memory::Address> GetRegionAndRelativeAddress(Address address);
	size_t GetSizeOfRegion(Region region);

	static constexpr size_t rom_bank_size_			{ 0x4000 };
	static constexpr size_t external_ram_bank_size_	{ 0x2000 };

#pragma region Region start addresses
	static constexpr Address isr_start_			{ 0x0040 };
	static constexpr Address tile_sets_start_	{ 0x8000 };
	static constexpr Address tile_map_0_start_	{ 0x9800 };
	static constexpr Address tile_map_1_start_	{ 0x9C00 };
	static constexpr Address external_ram_start_{ 0xA000 };
	static constexpr Address oam_start_			{ 0xFE00 };
	static constexpr Address io_region_start_	{ 0xFF00 };
#pragma endregion

#pragma region IO addresses
	static constexpr Address JOYP	{ 0xFF00 };

	// Timer
	static constexpr Address DIV	{ 0xFF04 };
	static constexpr Address TIMA	{ 0xFF05 };
	static constexpr Address TMA	{ 0xFF06 };
	static constexpr Address TAC	{ 0xFF07 };

	static constexpr Address IF		{ 0xFF0F };

	// APU
	static constexpr Address NR10	{ 0xFF10 };
	static constexpr Address NR11	{ 0xFF11 };
	static constexpr Address NR12	{ 0xFF12 };
	static constexpr Address NR13	{ 0xFF13 };
	static constexpr Address NR14	{ 0xFF14 };
	static constexpr Address NR21	{ 0xFF16 };
	static constexpr Address NR22	{ 0xFF17 };
	static constexpr Address NR23	{ 0xFF18 };
	static constexpr Address NR24	{ 0xFF19 };
	static constexpr Address NR30	{ 0xFF1A };
	static constexpr Address NR31	{ 0xFF1B };
	static constexpr Address NR32	{ 0xFF1C };
	static constexpr Address NR33	{ 0xFF1D };
	static constexpr Address NR34	{ 0xFF1E };
	static constexpr Address NR41	{ 0xFF20 };
	static constexpr Address NR42	{ 0xFF21 };
	static constexpr Address NR43	{ 0xFF22 };
	static constexpr Address NR44	{ 0xFF23 };
	static constexpr Address NR50	{ 0xFF24 };
	static constexpr Address NR51	{ 0xFF25 };
	static constexpr Address NR52	{ 0xFF26 };

	// GPU
	static constexpr Address LCDC	{ 0xFF40 };
	static constexpr Address STAT	{ 0xFF41 };
	static constexpr Address SCY	{ 0xFF42 };
	static constexpr Address SCX	{ 0xFF43 };
	static constexpr Address LY		{ 0xFF44 };
	static constexpr Address LYC	{ 0xFF45 };
	static constexpr Address DMA	{ 0xFF46 };
	static constexpr Address BGP	{ 0xFF47 };
	static constexpr Address OBP0	{ 0xFF48 };
	static constexpr Address OBP1	{ 0xFF49 };
	static constexpr Address WY		{ 0xFF4A };
	static constexpr Address WX		{ 0xFF4B };

	static constexpr Address IE		{ 0xFFFF };
#pragma endregion
}
