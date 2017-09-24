#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <array>
#include <tuple>

namespace Memory
{
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
		Interrupts,		// 0xFFFF - 0xFFFF
		Count
	};

	using Address = uint16_t;
	using Map = std::array<uint8_t, std::numeric_limits<uint16_t>::max() + 1>;

	Region GetRegion(Address address);

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

		bool operator==(const Watchpoint &rhs) { return (address == rhs.address) && (type == rhs.type); }
		friend bool operator<(const Watchpoint &lhs, const Watchpoint &rhs)
		{
			return (lhs.address < rhs.address) || ((lhs.address == rhs.address) && (static_cast<size_t>(lhs.type) < static_cast<size_t>(rhs.type)));
		}
	};

	// Memory region sizes
	static constexpr size_t rom_bank_size_			{ 0x4000 };
	static constexpr size_t vram_size_				{ 0x2000 };
	static constexpr size_t external_ram_bank_size_	{ 0x2000 };
	static constexpr size_t wram_size_				{ 0x2000 };
	static constexpr size_t wram_echo_size_			{ 0x1E00 };
	static constexpr size_t oam_size_				{ 0x00A0 };
	static constexpr size_t unused_region_size_		{ 0x0060 };
	static constexpr size_t io_region_size_			{ 0x0080 };
	static constexpr size_t hram_size_				{ 0x007F };
	static constexpr size_t interrupts_region_size_	{ 0x0001 };

	static constexpr Address ISR	{ 0x0040 };
	static constexpr Address IO		{ 0xFF00 };

	static constexpr Address rom_bank_n_offset_{ 0x4000 };
	static constexpr Address vram_offset_{ 0x8000 };
	static constexpr Address eram_offset_{ 0xA000 };
	static constexpr Address wram_offset_{ 0xC000 };
	static constexpr Address wram_echo_offset_{ 0xE000 };
	static constexpr Address oam_offset_{ 0xFE00 };
	static constexpr Address unused_memory_offset_{ 0xFEA0 };
	static constexpr Address io_offset_{ 0xFF00 };
	static constexpr Address hram_offset_{ 0xFF80 };
	static constexpr Address interrupts_offset_{ 0xFFFF };

	// Joypad
	static constexpr Address JOYP	{ 0xFF00 };

	// Timer
	static constexpr Address DIV	{ 0xFF04 };
	static constexpr Address TIMA	{ 0xFF05 };
	static constexpr Address TMA	{ 0xFF06 };
	static constexpr Address TAC	{ 0xFF07 };

	// CPU Interrupts
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
	static constexpr Address WaveStart	{ 0xFF30 };
	static constexpr Address WaveEnd	{ 0xFF3F };

	// PPU
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

	// CPU Interrupts
	static constexpr Address IE		{ 0xFFFF };
}
