#include "Memory.h"
#include <string>

namespace Memory
{
	/*std::pair<Region, Memory::Address> GetRegionAndRelativeAddress(Address address)
	{
		switch (address & 0xF000)
		{
		case 0x0000:
		case 0x1000:
		case 0x2000:
		case 0x3000:
			return std::make_pair(Region::ROM_Bank0, address);
		case 0x4000:
		case 0x5000:
		case 0x6000:
		case 0x7000:
			return std::make_pair(Region::ROM_OtherBanks, address & 0x3FFF);
		case 0x8000:
		case 0x9000:
			return std::make_pair(Region::VRAM, address & 0x1FFF);
		case 0xA000:
		case 0xB000:
			return std::make_pair(Region::ERAM, address & 0x1FFF);
		case 0xC000:
		case 0xD000:
			return std::make_pair(Region::WRAM, address & 0x1FFF);
		case 0xE000:
			return std::make_pair(Region::WRAM_Echo, address & 0x1FFF);
		case 0xF000:
			switch (address & 0x0F00)
			{
			default:
				return std::make_pair(Region::WRAM_Echo, address & 0x1FFF);
			case 0x0E00:
				if (address < 0xFEA0)	return std::make_pair(Region::OAM, address & 0xFF);
				else					return std::make_pair(Region::Unused, address & 0x7F);
			case 0x0F00:
				if (address < 0xFF80)		return std::make_pair(Region::IO, address & 0x7F);
				else if (address < 0xFFFF)	return std::make_pair(Region::HRAM, address & 0x7F);
				else						return std::make_pair(Region::Interrupts, address & 0x01);
			}
		default:
			throw std::logic_error{ "Error decoding region and relative address of absolute address " + std::to_string(address) };
		}
	}*/

	std::pair<Region, Memory::Address> GetRegionAndRelativeAddress(Address address)
	{
		if (address < 0x4000) { return std::make_pair(Region::ROM_Bank0, address); }
		else if (address < 0x8000) { return std::make_pair(Region::ROM_OtherBanks, address - 0x4000); }
		else if (address < 0xA000) { return std::make_pair(Region::VRAM, address - 0x8000); }
		else if (address < 0xC000) { return std::make_pair(Region::ERAM, address - 0xA000); }
		else if (address < 0xE000) { return std::make_pair(Region::WRAM, address - 0xC000); }
		else if (address < 0xFE00) { return std::make_pair(Region::WRAM_Echo, address - 0xE000); }
		else if (address < 0xFEA0) { return std::make_pair(Region::OAM, address - 0xFE00); }
		else if (address < 0xFF00) { return std::make_pair(Region::Unused, address - 0xFEA0); }
		else if (address < 0xFF80) { return std::make_pair(Region::IO, address - 0xFF00); }
		else if (address < 0xFFFF) { return std::make_pair(Region::HRAM, address - 0xFF80); }
		else { return std::make_pair(Region::Interrupts, address - 0xFFFF); }
	}

	bool operator<(const Memory::Watchpoint &lhs, const Memory::Watchpoint &rhs)
	{
		return (lhs.address < rhs.address) || ((lhs.address == rhs.address) && (static_cast<size_t>(lhs.type) < static_cast<size_t>(rhs.type)));
	}
}
