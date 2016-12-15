#include "Mbc1.h"
#include "MMU.h"

Mbc1::Mbc1(MMU &mmu) : IMbc(mmu)
{

}

void Mbc1::OnRomWritten(Memory::Address address, uint8_t value)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
		mmu_->EnableExternalRam((value & 0x0F) == 0x0A);
		break;
	case 0x2000:
	case 0x3000:
		loaded_rom_bank_ = (loaded_rom_bank_ & 0x60) | (value & 0x1F);
		// Banks 0x00, 0x20, 0x40 and 0x60 are unavailable and point to the next bank instead
		if ((loaded_rom_bank_ & 0x1F) == 0) loaded_rom_bank_ += 1;
		mmu_->LoadRomBank(loaded_rom_bank_);
		break;
	case 0x4000:
	case 0x5000:
		if (is_ram_banking_mode_)
		{
			loaded_ram_bank_ = (value & 0x3);
			mmu_->LoadRamBank(loaded_ram_bank_);
		}
		else
		{
			loaded_rom_bank_ = (loaded_rom_bank_ & 0x1F) | (value & 0x60);
			mmu_->LoadRomBank(loaded_rom_bank_);
		}
		break;
	case 0x6000:
	case 0x7000:
		is_ram_banking_mode_ = ((value & 0x10) != 0);
		break;
	}
}
