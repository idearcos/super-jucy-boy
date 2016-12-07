#include "MMU.h"
#include "Mbc1.h"
#include <fstream>

MMU::MMU()
{
	Reset();
}

MMU::~MMU()
{
	for (auto& deregister_function : mbc_listener_deregister_functions_)
	{
		deregister_function();
	}
	mbc_listener_deregister_functions_.clear();
}

void MMU::Reset()
{
	memory_.clear();

	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::ROM_0), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::ROM_Other), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::VRAM), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::ERAM), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::WRAM), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::WRAM_Echo), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::OAM), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::Unused), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::IO), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::HRAM), 0);
	memory_.emplace_back(Memory::GetSizeOfRegion(Memory::Region::Interrupts), 0);

	WriteByte(0xFF05, 0x00); //TIMA
	WriteByte(0xFF06, 0x00); //TMA
	WriteByte(0xFF07, 0x00); //TAC
	WriteByte(0xFF10, 0x80); //NR10
	WriteByte(0xFF11, 0xBF); //NR11
	WriteByte(0xFF12, 0xF3); //NR12
	WriteByte(0xFF14, 0xBF); //NR14
	WriteByte(0xFF16, 0x3F); //NR21
	WriteByte(0xFF17, 0x00); //NR22
	WriteByte(0xFF19, 0xBF); //NR24
	WriteByte(0xFF1A, 0x7F); //NR30
	WriteByte(0xFF1B, 0xFF); //NR31
	WriteByte(0xFF1C, 0x9F); //NR32
	WriteByte(0xFF1E, 0xBF); //NR33
	WriteByte(0xFF20, 0xFF); //NR41
	WriteByte(0xFF21, 0x00); //NR42
	WriteByte(0xFF22, 0x00); //NR43
	WriteByte(0xFF23, 0xBF); //NR30
	WriteByte(0xFF24, 0x77); //NR50
	WriteByte(0xFF25, 0xF3); //NR51
	WriteByte(0xFF26, 0xF1); //NR52
	WriteByte(0xFF40, 0x91); //LCDC
	WriteByte(0xFF42, 0x00); //SCY
	WriteByte(0xFF43, 0x00); //SCX
	WriteByte(0xFF45, 0x00); //LYC
	WriteByte(0xFF47, 0xFC); //BGP
	WriteByte(0xFF48, 0xFF); //OBP0
	WriteByte(0xFF49, 0xFF); //OBP1
	WriteByte(0xFF4A, 0x00); //WY
	WriteByte(0xFF4B, 0x00); //WX
	WriteByte(0xFFFF, 0x00); //IE
}

uint8_t MMU::ReadByte(Memory::Address address) const
{
	const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

	return memory_[region_and_relative_address.first][region_and_relative_address.second];
}

uint16_t MMU::ReadWord(Memory::Address address) const
{
	uint16_t value{ ReadByte(address) };
	value += (ReadByte(++address) << 8);
	return value; 
}

void MMU::WriteByte(Memory::Address address, uint8_t value, bool notify)
{
	const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

	switch (region_and_relative_address.first)
	{
	case Memory::Region::ROM_0:
	case Memory::Region::ROM_Other:
		if (notify) NotifyMemoryWrite(region_and_relative_address.first, address, value);
		return;
	case Memory::Region::VRAM:
		//TODO: ignore writes during VRAM GPU state
		break;
	case Memory::Region::ERAM:
		if (!external_ram_enabled_) return;
		break;
	case Memory::Region::OAM:
		//TODO: ignore writes during OAM and VRAM GPU states
		break;
	default:
		break;
	}

	memory_[region_and_relative_address.first][region_and_relative_address.second] = value;

	if (notify) NotifyMemoryWrite(region_and_relative_address.first, address, value);
}

void MMU::WriteWord(Memory::Address address, uint16_t value, bool notify)
{
	WriteByte(address, (value & 0xFF), notify);
	WriteByte(++address, ((value >> 8) & 0xFF), notify);
}

void MMU::LoadRom(const std::string &rom_file_path)
{
	rom_loaded_ = false;
	rom_banks.clear();
	for (auto& deregister_function : mbc_listener_deregister_functions_)
	{
		deregister_function();
	}
	mbc_listener_deregister_functions_.clear();

	std::ifstream rom_read_stream{ rom_file_path, std::ios::binary | std::ios::ate };
	if (!rom_read_stream.is_open()) { throw std::runtime_error{ "ROM file could not be opened" }; }

	const auto file_size = static_cast<size_t>(rom_read_stream.tellg());
	rom_read_stream.seekg(0, std::ios::beg);

	//TODO: check byte 0x147 for number of ROM banks
	for (auto i = 0; i < file_size / Memory::GetSizeOfRegion(Memory::ROM_0); ++i)
	{
		rom_banks.emplace_back();
		rom_banks.back().resize(Memory::GetSizeOfRegion(Memory::ROM_0));
		rom_read_stream.read(reinterpret_cast<char*>(rom_banks.back().data()), rom_banks.back().size());
	}

	// Map ROM banks 0 and 1
	memory_[Memory::ROM_0].swap(rom_banks[0]);
	memory_[Memory::ROM_Other].swap(rom_banks[1]);

	rom_read_stream.close();

	rom_loaded_ = true;
	loaded_rom_bank_ = 1;

	// Create the appropriate MBC
	switch (memory_[Memory::ROM_0][0x147])
	{
	case 0:
		// No MBC
		break;
	case 1:
	case 2:
	case 3:
		mbc_ = std::make_unique<Mbc1>(*this);
		mbc_listener_deregister_functions_.emplace_back(AddListener(*mbc_, &IMbc::OnRomWritten, Memory::Region::ROM_0));
		mbc_listener_deregister_functions_.emplace_back(AddListener(*mbc_, &IMbc::OnRomWritten, Memory::Region::ROM_Other));
		break;
	}
	
	// Create the necessary number of RAM banks and map the first one
	size_t num_ram_banks{ 0 };
	switch (memory_[Memory::ROM_0][0x149])
	{
		//TODO: MBC2 uses value 0
	case 0:
		num_ram_banks = 0;
		break;
	case 1:
	case 2:
		//TODO: case 1 is only a 2 kB bank
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
	}

	if (num_ram_banks > 0)
	{
		for (int i = 0; i < num_ram_banks; ++i)
		{
			external_ram_banks.emplace_back(size_t{ 0x2000 }, uint8_t{ 0 });
		}

		memory_[Memory::ERAM].swap(external_ram_banks[0]);
		loaded_eram_bank_ = 0;
	}
}

void MMU::LoadRomBank(size_t rom_bank_number)
{
	// Swap the currently loaded ROM bank back into its original slot
	memory_[Memory::ROM_Other].swap(rom_banks[loaded_rom_bank_]);
	memory_[Memory::ROM_Other].swap(rom_banks[rom_bank_number]);
	loaded_rom_bank_ = rom_bank_number;
}

void MMU::LoadRamBank(size_t ram_bank_number)
{
	// Swap the currently loaded RAM bank back into its original slot
	memory_[Memory::ERAM].swap(rom_banks[loaded_eram_bank_]);
	memory_[Memory::ERAM].swap(rom_banks[ram_bank_number]);
	loaded_eram_bank_ = ram_bank_number;
}

Memory::Map MMU::GetMemoryMap() const
{
	Memory::Map memory_map{};
	size_t offset{ 0 };

	for (const auto& memory_region : memory_)
	{
		memcpy(memory_map.data() + offset, memory_region.data(), memory_region.size());
		offset += memory_region.size();
	}
	
	return memory_map;
}

#pragma region Listener notification
void MMU::NotifyMemoryWrite(Memory::Region region, Memory::Address address, uint8_t value)
{
	for (auto& listener : listeners_[region])
	{
		listener(address, value);
	}
}
#pragma endregion
