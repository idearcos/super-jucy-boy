#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "Memory.h"

class Cartridge final
{
public:
	Cartridge(const std::string &rom_file_path);
	~Cartridge() = default;

	// MMU mapped memory read/write functions
	uint8_t OnRomBank0Read(Memory::Address relative_address) const;
	void OnRomBank0Written(Memory::Address relative_address, uint8_t value);
	uint8_t OnRomBankNRead(Memory::Address relative_address) const;
	void OnRomBankNWritten(Memory::Address relative_address, uint8_t value);
	uint8_t OnExternalRamRead(Memory::Address relative_address) const;
	void OnExternalRamWritten(Memory::Address relative_address, uint8_t value);

	size_t GetLoadedRamBank() const { return loaded_external_ram_bank_; }
	void LoadRomBank(size_t rom_bank_number);
	void LoadRamBank(size_t external_ram_bank_number);

private:
	static size_t GetRomSize(uint8_t rom_size_code);

	inline void OnNoMbcWritten(Memory::Address, uint8_t) { return; }
	void OnMbc1Written(Memory::Address address, uint8_t value);

private:
	std::vector<std::vector<uint8_t>> rom_banks_;
	std::vector<std::vector<uint8_t>> external_ram_banks_;

	std::function<void(Memory::Address, uint8_t)> mbc_write_function_;

	size_t loaded_rom_bank_{ 1 };
	size_t loaded_external_ram_bank_{ 0 };
	bool external_ram_enabled_{ false };

	// MBC implementation members
	bool mbc1_ram_banking_mode_enabled_{ false };
};
