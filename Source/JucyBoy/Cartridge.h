#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include "Memory.h"

class Cartridge final
{
public:
	Cartridge(const std::string &rom_file_path);
	~Cartridge();

	// MMU mapped memory read/write functions
	uint8_t OnRomBank0Read(const Memory::Address &address) const;
	void OnRomBank0Written(const Memory::Address &address, uint8_t value);
	uint8_t OnRomBankNRead(const Memory::Address &address) const;
	void OnRomBankNWritten(const Memory::Address &address, uint8_t value);
	uint8_t OnExternalRamRead(const Memory::Address &address) const;
	void OnExternalRamWritten(const Memory::Address &address, uint8_t value);

	template<class Archive>
	void serialize(Archive &archive);

private:
	static size_t GetNumRomBanks(uint8_t rom_size_code);
	static std::string GetMbcType(uint8_t cartridge_type_code);
	static bool HasExternalBattery(uint8_t cartridge_type_code);

	inline void OnNoMbcWritten(const Memory::Address&, uint8_t) { return; }
	void OnMbc1Written(const Memory::Address &address, uint8_t value);

	void UpdateSelectedBanks();
	inline size_t GetRomBankSelectionMask() const { return rom_banks_.size() - 1; } // ToDo: change mask logic when supporting 72, 80 and 96 bank ROMs
	inline size_t GetRamBankSelectionMask() const { return external_ram_banks_.empty() ? 0 : external_ram_banks_.size() - 1; }

private:
	std::vector<std::vector<uint8_t>> rom_banks_;
	std::vector<std::vector<uint8_t>> external_ram_banks_;

	std::function<void(const Memory::Address&, uint8_t)> mbc_write_function_;

	size_t bank_selection_value_{ 0 }; // Unaltered value written in the 7 bank selection pins
	size_t selected_rom_bank_0_{ 0 }; // Bank accessed in the ROM bank 0 region (0x0000 - 0x3FFF)
	size_t selected_rom_bank_N_{ 1 }; // Bank accessed in the ROM bank N region (0x4000 - 0x7FFF)
	size_t selected_external_ram_bank_{ 0 };
	bool external_ram_enabled_{ false };

	// MBC implementation members
	bool mbc1_ram_banking_mode_enabled_{ false };

	std::string eram_save_file_path_;
};

template<class Archive>
void Cartridge::serialize(Archive &archive)
{
	archive(external_ram_banks_);
	archive(bank_selection_value_, selected_rom_bank_0_, selected_rom_bank_N_, selected_external_ram_bank_, external_ram_enabled_, mbc1_ram_banking_mode_enabled_);
}
