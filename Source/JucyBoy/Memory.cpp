#include "Memory.h"

namespace Memory
{
	Region GetRegion(Address address)
	{
		if (address < rom_bank_n_offset_)			return Region::ROM_Bank0;
		else if (address < vram_offset_)			return Region::ROM_OtherBanks;
		else if (address < eram_offset_)			return Region::VRAM;
		else if (address < wram_offset_)			return Region::ERAM;
		else if (address < wram_echo_offset_)		return Region::WRAM;
		else if (address < oam_offset_)				return Region::WRAM_Echo;
		else if (address < unused_memory_offset_)	return Region::OAM;
		else if (address < io_offset_)				return Region::Unused;
		else if (address < hram_offset_)			return Region::IO;
		else if (address < interrupts_offset_)		return Region::HRAM;
		else										return Region::Interrupts;
	}
}
