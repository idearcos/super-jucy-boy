#include "Memory.h"

namespace Memory
{
	Region GetRegionOfAddress(Address address)
	{
		if (address < 0x4000)		{ return Region::ROM_0; }
		else if (address < 0x8000)	{ return Region::ROM_Other; }
		else if (address < 0xA000)	{ return Region::VRAM; }
		else if (address < 0xC000)	{ return Region::ERAM; }
		else if (address < 0xE000)	{ return Region::WRAM; }
		else if (address < 0xFE00)	{ return Region::WRAM_Echo; }
		else if (address < 0xFEA0)	{ return Region::OAM; }
		else if (address < 0xFF00)	{ return Region::None; }
		else if (address < 0xFF80)	{ return Region::IO; }
		else if (address < 0xFFFF)	{ return Region::HRAM; }
		else						{ return Region::Interrupts; }
	}
}
