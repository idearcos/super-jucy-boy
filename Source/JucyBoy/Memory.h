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

	class Address
	{
	public:
		constexpr Address(uint16_t absolute) : Address(absolute, AbsoluteToRegionAndRelative(absolute)) {}
		constexpr Address() : Address(0) {}

		inline uint16_t GetAbsolute() const { return absolute_; }
		inline Region GetRegion() const { return region_; }
		inline uint16_t GetRelative() const { return relative_; }

		constexpr explicit operator uint16_t() const { return absolute_; }

		Address& operator+=(const uint16_t rhs)
		{
			absolute_ += rhs;
			std::tie(region_, relative_) = AbsoluteToRegionAndRelative(absolute_);
			return *this;
		}
		friend Address operator+(Address lhs, uint16_t rhs) { lhs += rhs; return lhs; }
		
		Address& operator-=(const uint16_t rhs)
		{
			absolute_ -= rhs;
			std::tie(region_, relative_) = AbsoluteToRegionAndRelative(absolute_);
			return *this;
		}
		friend Address operator-(Address lhs, uint16_t rhs) { lhs -= rhs; return lhs; }

		Address& operator++()
		{
			++absolute_;
			std::tie(region_, relative_) = AbsoluteToRegionAndRelative(absolute_);
			return *this;
		}
		Address operator++(int)
		{
			Address tmp(*this);
			operator++();
			return tmp;
		}

		Address& operator&=(uint16_t rhs)
		{
			absolute_ &= rhs;
			std::tie(region_, relative_) = AbsoluteToRegionAndRelative(absolute_);
			return *this;
		}
		friend Address operator&(Address lhs, uint16_t rhs) { lhs &= rhs; return lhs; }

		friend bool operator==(const Address& lhs, const Address& rhs) { return lhs.absolute_ == rhs.absolute_; }
		friend bool operator!=(const Address& lhs, const Address& rhs) { return !(lhs == rhs); }

		friend bool operator< (const Address& lhs, const Address& rhs) { return lhs.absolute_ < rhs.absolute_; }
		friend bool operator> (const Address& lhs, const Address& rhs) { return rhs < lhs; }
		friend bool operator<=(const Address& lhs, const Address& rhs) { return !(lhs > rhs); }
		friend bool operator>=(const Address& lhs, const Address& rhs) { return !(lhs < rhs); }

		template<class Archive>
		void serialize(Archive &archive)
		{
			archive(absolute_);
			std::tie(region_, relative_) = AbsoluteToRegionAndRelative(absolute_);
		}

	private:
		constexpr Address(uint16_t absolute, std::tuple<Region, uint16_t> region_and_relative) :
			absolute_{ absolute },
			region_{ std::get<0>(region_and_relative) },
			relative_{ std::get<1>(region_and_relative) }
		{}

		constexpr static std::tuple<Region, uint16_t> AbsoluteToRegionAndRelative(uint16_t absolute)
		{
			if (absolute < 0x4000)		return std::make_tuple(Region::ROM_Bank0, absolute);
			else if (absolute < 0x8000) return std::make_tuple(Region::ROM_OtherBanks, static_cast<uint16_t>(absolute - 0x4000));
			else if (absolute < 0xA000) return std::make_tuple(Region::VRAM, static_cast<uint16_t>(absolute - 0x8000));
			else if (absolute < 0xC000) return std::make_tuple(Region::ERAM, static_cast<uint16_t>(absolute - 0xA000));
			else if (absolute < 0xE000) return std::make_tuple(Region::WRAM, static_cast<uint16_t>(absolute - 0xC000));
			else if (absolute < 0xFE00) return std::make_tuple(Region::WRAM_Echo, static_cast<uint16_t>(absolute - 0xE000));
			else if (absolute < 0xFEA0) return std::make_tuple(Region::OAM, static_cast<uint16_t>(absolute - 0xFE00));
			else if (absolute < 0xFF00) return std::make_tuple(Region::Unused, static_cast<uint16_t>(absolute - 0xFEA0));
			else if (absolute < 0xFF80) return std::make_tuple(Region::IO, static_cast<uint16_t>(absolute - 0xFF00));
			else if (absolute < 0xFFFF) return std::make_tuple(Region::HRAM, static_cast<uint16_t>(absolute - 0xFF80));
			else						return std::make_tuple(Region::Interrupts, static_cast<uint16_t>(absolute - 0xFFFF));
		}

	private:
		uint16_t absolute_;
		Region region_;
		uint16_t relative_;
	};

	using Map = std::array<uint8_t, std::numeric_limits<uint16_t>::max() + 1>;

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

	static constexpr Address isr_start_			{ 0x0040 };
	static constexpr Address io_region_start_	{ 0xFF00 };

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
