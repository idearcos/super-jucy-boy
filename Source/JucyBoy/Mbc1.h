#pragma once

#include "IMbc.h"

class MMU;

class Mbc1 final : public IMbc
{
public:
	Mbc1(MMU &mmu);
	~Mbc1() = default;

	void OnRomWritten(Memory::Address address, uint8_t value) override;

private:
	bool is_ram_banking_mode_{ false };
	size_t loaded_rom_bank_{ 1 };
	size_t loaded_ram_bank_{ 0 };

	MMU* mmu_;
};
