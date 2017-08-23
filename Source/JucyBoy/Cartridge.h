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
	uint8_t OnRomBank0Read(const Memory::Address &address) const;
	void OnRomBank0Written(const Memory::Address &address, uint8_t value);
	uint8_t OnRomBankNRead(const Memory::Address &address) const;
	void OnRomBankNWritten(const Memory::Address &address, uint8_t value);
	uint8_t OnExternalRamRead(const Memory::Address &address) const;
	void OnExternalRamWritten(const Memory::Address &address, uint8_t value);

	size_t GetLoadedRamBank() const { return loaded_external_ram_bank_; }
	void LoadRomBank(size_t rom_bank_number);
	void LoadRamBank(size_t external_ram_bank_number);

	template<class Archive>
	void serialize(Archive &archive);

private:
	static size_t GetNumRomBanks(uint8_t rom_size_code);

	inline void OnNoMbcWritten(const Memory::Address&, uint8_t) { return; }
	void OnMbc1Written(const Memory::Address &address, uint8_t value);

private:
	std::vector<std::vector<uint8_t>> rom_banks_;
	std::vector<std::vector<uint8_t>> external_ram_banks_;

	std::function<void(const Memory::Address&, uint8_t)> mbc_write_function_;

	size_t loaded_rom_bank_{ 1 };
	size_t rom_bank_selection_mask{ 0 };
	size_t loaded_external_ram_bank_{ 0 };
	size_t external_ram_bank_selection_mask{ 0 };
	bool external_ram_enabled_{ false };

	// MBC implementation members
	bool mbc1_ram_banking_mode_enabled_{ false };
};

template<class Archive>
void Cartridge::serialize(Archive &archive)
{
	archive(external_ram_banks_);
	archive(loaded_rom_bank_, loaded_external_ram_bank_, external_ram_enabled_, mbc1_ram_banking_mode_enabled_);
}
