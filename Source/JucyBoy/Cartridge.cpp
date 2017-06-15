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
	const auto rom_size = GetRomSize(file_header[0x148]);
	if (file_size != rom_size) throw std::invalid_argument{ "ROM file size (" + std::to_string(file_size) + " bytes) is not the same as the ROM size according to header (" + std::to_string(rom_size) + " bytes)" };

	// Create ROM banks
	for (auto i = 0; i < file_size / Memory::rom_bank_size_; ++i)
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
		throw std::logic_error{ "Unsupported MBC: " + std::to_string(static_cast<int>(file_header[0x147])) };
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
}

#pragma region MMU mapped memory read/write functions
uint8_t Cartridge::OnRomBank0Read(Memory::Address relative_address) const
{
	return rom_banks_[0][relative_address];
}

void Cartridge::OnRomBank0Written(Memory::Address relative_address, uint8_t value)
{
	mbc_write_function_(relative_address, value);
}

uint8_t Cartridge::OnRomBankNRead(Memory::Address relative_address) const
{
	return rom_banks_[loaded_rom_bank_][relative_address];
}

void Cartridge::OnRomBankNWritten(Memory::Address relative_address, uint8_t value)
{
	mbc_write_function_(relative_address + Memory::rom_bank_size_, value);
}

uint8_t Cartridge::OnExternalRamRead(Memory::Address relative_address) const
{
	if (!external_ram_enabled_) return 0xFF;
	if (external_ram_banks_.empty()) return 0xFF;

	if (relative_address >= external_ram_banks_[loaded_external_ram_bank_].size()) throw std::out_of_range{ "Trying to read from out of external RAM range" };

	return external_ram_banks_[loaded_external_ram_bank_][relative_address];
}

void Cartridge::OnExternalRamWritten(Memory::Address relative_address, uint8_t value)
{
	if (!external_ram_enabled_) return;
	if (external_ram_banks_.empty()) return;

	if (relative_address >= external_ram_banks_[loaded_external_ram_bank_].size()) return;

	external_ram_banks_[loaded_external_ram_bank_][relative_address] = value;
}
#pragma endregion

void Cartridge::LoadRomBank(size_t rom_bank_number)
{
	assert(rom_bank_number != 0);
	if (rom_bank_number >= rom_banks_.size()) throw std::invalid_argument("Requested invalid ROM bank: " + std::to_string(rom_bank_number));

	loaded_rom_bank_ = rom_bank_number;
}

void Cartridge::LoadRamBank(size_t external_ram_bank_number)
{
	if (external_ram_bank_number >= external_ram_banks_.size()) throw std::invalid_argument("Requested invalid external RAM bank: " + std::to_string(external_ram_bank_number));

	loaded_external_ram_bank_ = external_ram_bank_number;
}

size_t Cartridge::GetRomSize(uint8_t rom_size_code)
{
	switch (rom_size_code)
	{
	case 0x00: return 2 * Memory::rom_bank_size_; // 0x8000
	case 0x01: return 4 * Memory::rom_bank_size_; // 0x10000
	case 0x02: return 8 * Memory::rom_bank_size_; // 0x20000
	case 0x03: return 16 * Memory::rom_bank_size_; // 0x40000
	case 0x04: return 32 * Memory::rom_bank_size_; // 0x80000
	case 0x05: return 64 * Memory::rom_bank_size_; // 0x100000
	case 0x06: return 128 * Memory::rom_bank_size_; // 0x200000
	case 0x07: return 256 * Memory::rom_bank_size_; // 0x400000
	case 0x08: return 512 * Memory::rom_bank_size_; // 0x800000
	case 0x52: return 72 * Memory::rom_bank_size_; // 0x120000
	case 0x53: return 80 * Memory::rom_bank_size_; // 0x140000
	case 0x54: return 96 * Memory::rom_bank_size_; // 0x180000

	default: throw std::invalid_argument{ "Unsupported ROM size code: " + std::to_string(static_cast<int>(rom_size_code)) };
	}
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
	{auto rom_bank_to_load = (loaded_rom_bank_ & 0x60) | (value & 0x1F);

	// Banks 0x00, 0x20, 0x40 and 0x60 are unavailable and point to the next bank instead
	if ((rom_bank_to_load & 0x1F) == 0) rom_bank_to_load += 1;
	LoadRomBank(rom_bank_to_load); }
	break;
	case 0x4000:
	case 0x5000:
		if (mbc1_ram_banking_mode_enabled_)
		{
			LoadRamBank(value & 0x03);
		}
		else
		{
			LoadRomBank((loaded_rom_bank_ & 0x1F) | (value & 0x60));
		}
		break;
	case 0x6000:
	case 0x7000:
		mbc1_ram_banking_mode_enabled_ = ((value & 0x01) != 0);
		break;
	}
}
#pragma endregion
