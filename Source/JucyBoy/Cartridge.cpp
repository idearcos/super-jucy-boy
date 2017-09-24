#include "Cartridge.h"
#include <cassert>
#include <fstream>
#include <array>

Cartridge::Cartridge(const std::string &rom_file_path)
{
	std::ifstream rom_read_stream{ rom_file_path, std::ios::binary | std::ios::ate };
	if (!rom_read_stream.is_open()) { throw std::runtime_error{ "ROM file could not be opened" }; }

	const auto file_size = static_cast<size_t>(rom_read_stream.tellg());
	rom_read_stream.seekg(0, std::ios::beg);

	// Read ROM file header
	std::array<char, 0x150> file_header;
	if (file_size < file_header.size()) throw std::invalid_argument{ "ROM file (" + std::to_string(file_size) + " bytes) is smaller than header size (336 bytes)" };
	
	rom_read_stream.read(file_header.data(), file_header.size());
	rom_read_stream.seekg(0, std::ios::beg);

	// Get number of ROM banks according to header
	const auto num_rom_banks = GetNumRomBanks(file_header[0x148]);
	if (file_size != (num_rom_banks * Memory::rom_bank_size_)) throw std::invalid_argument{ "ROM file size (" + std::to_string(file_size) + " bytes) is not the same as the ROM size according to header (" + std::to_string(num_rom_banks) + " banks of 16384 bytes)" };

	// Create ROM banks
	for (auto ii = 0; ii < num_rom_banks; ++ii)
	{
		rom_banks_.emplace_back();
		rom_banks_.back().resize(Memory::rom_bank_size_);
		rom_read_stream.read(reinterpret_cast<char*>(rom_banks_.back().data()), rom_banks_.back().size());
	}

	rom_read_stream.close();

	// Create the appropriate MBC
	switch (file_header[0x147])
	{
	case 0:
		mbc_write_function_ = std::bind(&Cartridge::OnNoMbcWritten, this, std::placeholders::_1, std::placeholders::_2);
		break;
	case 1:
	case 2:
	case 3:
		mbc_write_function_ = std::bind(&Cartridge::OnMbc1Written, this, std::placeholders::_1, std::placeholders::_2);
		break;
	default:
		throw std::logic_error{ "Unsupported cartridge type: " + GetMbcType(file_header[0x147]) };
	}

	// Create the necessary number of external RAM banks
	size_t num_ram_banks{ 0 };
	switch (file_header[0x149])
	{
	case 0:
		num_ram_banks = 0;
		break;
	case 1:
		// Special case: single RAM bank of 2 kBytes
		external_ram_banks_.emplace_back(0x800, uint8_t{ 0 });
		break;
	case 2:
		num_ram_banks = 1;
		break;
	case 3:
		num_ram_banks = 4;
		break;
	case 4:
		num_ram_banks = 16;
		break;
	case 5:
		num_ram_banks = 8;
		break;
	default:
		throw std::invalid_argument{ "Unsupported external RAM size code: " + std::to_string(static_cast<int>(file_header[0x149])) };
	}

	for (int i = 0; i < num_ram_banks; ++i)
	{
		external_ram_banks_.emplace_back(Memory::external_ram_bank_size_, uint8_t{ 0 });
	}

	// Verify whether the cartridge has an external battery to load RAM state file
	const auto has_external_battery = HasExternalBattery(file_header[0x147]);
	if (has_external_battery)
	{
		eram_save_file_path_ = rom_file_path;
		const auto last_dot_position = eram_save_file_path_.find_last_of('.');
		if (std::string::npos != last_dot_position)
		{
			eram_save_file_path_.replace(last_dot_position, std::string::npos, ".sav");
		}
		else
		{
			eram_save_file_path_.append(".sav");
		}

		std::ifstream eram_sav_file{ eram_save_file_path_, std::ios::binary | std::ios::ate };
		if (!eram_sav_file.is_open()) return;
		
		const auto file_size = static_cast<size_t>(eram_sav_file.tellg());
		size_t total_eram_size{ 0 };
		for (const auto &eram_bank : external_ram_banks_)
		{
			total_eram_size += eram_bank.size();
		}
		if (file_size != total_eram_size) return;

		eram_sav_file.seekg(0, std::ios::beg);
		for (auto &eram_bank : external_ram_banks_)
		{
			eram_sav_file.read(reinterpret_cast<char*>(eram_bank.data()), eram_bank.size());
		}
	}
}

Cartridge::~Cartridge()
{
	// Verify whether the cartridge has an external battery to save RAM state file
	if (!eram_save_file_path_.empty())
	{
		std::ofstream eram_sav_file{ eram_save_file_path_, std::ios::binary | std::ios::trunc };
		if (!eram_sav_file.is_open()) return;

		for (auto &eram_bank : external_ram_banks_)
		{
			eram_sav_file.write(reinterpret_cast<char*>(eram_bank.data()), eram_bank.size());
		}
	}
}

#pragma region MMU mapped memory read/write functions
uint8_t Cartridge::OnRomBank0Read(Memory::Address address) const
{
	return rom_banks_[selected_rom_bank_0_][address];
}

void Cartridge::OnRomBank0Written(Memory::Address address, uint8_t value)
{
	mbc_write_function_(address, value);
}

uint8_t Cartridge::OnRomBankNRead(Memory::Address address) const
{
	return rom_banks_[selected_rom_bank_N_][address - Memory::rom_bank_n_offset_];
}

void Cartridge::OnRomBankNWritten(Memory::Address address, uint8_t value)
{
	mbc_write_function_(address, value);
}

uint8_t Cartridge::OnExternalRamRead(Memory::Address address) const
{
	if (!external_ram_enabled_) return 0xFF;
	if (external_ram_banks_.empty()) return 0xFF;

	if ((address - Memory::eram_offset_) >= external_ram_banks_[selected_external_ram_bank_].size()) throw std::out_of_range{ "Trying to read from out of external RAM range" };

	return external_ram_banks_[selected_external_ram_bank_][address - Memory::eram_offset_];
}

void Cartridge::OnExternalRamWritten(Memory::Address address, uint8_t value)
{
	if (!external_ram_enabled_) return;
	if (external_ram_banks_.empty()) return;

	if ((address - Memory::eram_offset_) >= external_ram_banks_[selected_external_ram_bank_].size()) return;

	external_ram_banks_[selected_external_ram_bank_][address - Memory::eram_offset_] = value;
}
#pragma endregion

void Cartridge::UpdateSelectedBanks()
{
	// In RAM banking mode, the lower region can also point to banks 0x20, 0x40 and 0x60
	selected_rom_bank_0_ = mbc1_ram_banking_mode_enabled_ ? ((bank_selection_value_ & 0x60) & GetRomBankSelectionMask()) : 0;

	selected_rom_bank_N_ = bank_selection_value_ & GetRomBankSelectionMask();

	// For banks 0x00, 0x20, 0x40 and 0x60 the high region points to the next bank instead
	if ((bank_selection_value_ & 0x1F) == 0)
	{
		selected_rom_bank_N_ += 1;
	}

	selected_external_ram_bank_ = mbc1_ram_banking_mode_enabled_ ? (((bank_selection_value_ >> 5) & GetRamBankSelectionMask())) : 0;
}

#pragma region MBC implementations
void Cartridge::OnMbc1Written(Memory::Address address, uint8_t value)
{
	switch (address & 0xF000)
	{
	case 0x0000:
	case 0x1000:
		external_ram_enabled_ = ((value & 0x0F) == 0x0A);
		break;
	case 0x2000:
	case 0x3000:
		// Writes the lower 5 bits
		bank_selection_value_ = (bank_selection_value_ & 0x60) | (value & 0x1F);
		UpdateSelectedBanks();
		break;
	case 0x4000:
	case 0x5000:
		// Writes the upper 2 bits
		bank_selection_value_ = (bank_selection_value_ & 0x1F) | ((value & 0x03) << 5);
		UpdateSelectedBanks();
		break;
	case 0x6000:
	case 0x7000:
		mbc1_ram_banking_mode_enabled_ = ((value & 0x01) != 0);

		// Correct the ROM/RAM memory banks for the new mode
		UpdateSelectedBanks();

		break;
	}
}
#pragma endregion

size_t Cartridge::GetNumRomBanks(uint8_t rom_size_code)
{
	switch (rom_size_code)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
		return 2 << rom_size_code;
		//case 0x52: return 72; // 0x120000 bytes
		//case 0x53: return 80; // 0x140000 bytes
		//case 0x54: return 96; // 0x180000 bytes

	default: throw std::invalid_argument{ "Unsupported ROM size code: " + std::to_string(static_cast<int>(rom_size_code)) };
	}
}

std::string Cartridge::GetMbcType(uint8_t cartridge_type_code)
{
	switch (cartridge_type_code)
	{
	case 0x00: return "ROM only";
	case 0x01: return "MBC1";
	case 0x02: return "MBC1 + RAM";
	case 0x03: return "MBC1 + RAM + Battery";
	case 0x05: return "MBC2";
	case 0x06: return "MBC2 + Battery";
	case 0x08: return "ROM + RAM";
	case 0x09: return "ROM + RAM + Battery";
	case 0x0B: return "MMM01";
	case 0x0C: return "MMM01 + RAM";
	case 0x0D: return "MMM01 + RAM + Battery";
	case 0x0F: return "MBC3 + Timer + Battery";
	case 0x10: return "MBC3 + Timer + RAM + Battery";
	case 0x11: return "MBC3";
	case 0x12: return "MBC3 + RAM";
	case 0x13: return "MBC3 + RAM + Battery";
	case 0x19: return "MBC5";
	case 0x1A: return "MBC5 + RAM";
	case 0x1B: return "MBC5 + RAM + Battery";
	case 0x1C: return "MBC5 + Rumble";
	case 0x1D: return "MBC5 + Rumble + RAM";
	case 0x1E: return "MBC5 + Rumble + RAM + Battery";
	case 0x20: return "MBC6";
	case 0x22: return "MBC7 + Sensor + Rumble + RAM + Battery";
	case 0xFC: return "Pocket Camera";
	case 0xFD: return "Bandai Tama5";
	case 0xFE: return "HuC3";
	case 0xFF: return "HuC1 + RAM + Battery";
	default: throw std::logic_error{ "Unknown cartridge type code: " + std::to_string(cartridge_type_code) };
	}
}

bool Cartridge::HasExternalBattery(uint8_t cartridge_type_code)
{
	switch (cartridge_type_code)
	{
	case 0x03:
	case 0x06:
	case 0x09:
	case 0x0D:
	case 0x0F:
	case 0x10:
	case 0x13:
	case 0x1B:
	case 0x1E:
	case 0x22:
	case 0xFF:
		return true;
	default:
		return false;
	}
}
