#include "Memory.h"

namespace Memory
{
std::pair<Region, Memory::Address> GetRegionAndRelativeAddress(Address address)
	{
		if (address < 0x4000)		{ return std::make_pair(Region::ROM_Bank0, address); }
		else if (address < 0x8000)	{ return std::make_pair(Region::ROM_OtherBanks, address - 0x4000); }
		else if (address < 0xA000)	{ return std::make_pair(Region::VRAM, address - 0x8000); }
		else if (address < 0xC000)	{ return std::make_pair(Region::ERAM, address - 0xA000); }
		else if (address < 0xE000)	{ return std::make_pair(Region::WRAM, address - 0xC000); }
		else if (address < 0xFE00)	{ return std::make_pair(Region::WRAM_Echo, address - 0xE000); }
		else if (address < 0xFEA0)	{ return std::make_pair(Region::OAM, address - 0xFE00); }
		else if (address < 0xFF00)	{ return std::make_pair(Region::Unused, address - 0xFEA0); }
		else if (address < 0xFF80)	{ return std::make_pair(Region::IO, address - 0xFF00); }
		else if (address < 0xFFFF)	{ return std::make_pair(Region::HRAM, address - 0xFF80); }
		else						{ return std::make_pair(Region::Interrupts, address - 0xFFFF); }
	}
}
