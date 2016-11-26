#include "MMU.h"
#include <fstream>

MMU::MMU()
{
	Reset();
}

MMU::~MMU()
{

}

void MMU::Reset()
{
	memory_[0xFF00] = 0xFE;

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
	return memory_[address];
}

uint16_t MMU::ReadWord(Memory::Address address) const
{
	uint16_t value{ ReadByte(address) };
	value += (ReadByte(++address) << 8);
	return value; 
}

void MMU::WriteByte(Memory::Address address, uint8_t value, bool notify)
{
	switch (Memory::GetRegionOfAddress(address))
	{
	case Memory::Region::ROM_0:
	case Memory::Region::ROM_Other:
		//TODO: notify MBC
		return;
	case Memory::Region::VRAM:
		//TODO: ignore writes during VRAM GPU state
		break;
	case Memory::Region::OAM:
		//TODO: ignore writes during OAM and VRAM GPU states
		break;
	case Memory::Region::IO:
		if (address == 0xFF00) return;
	}

	memory_[address] = value;

	if (notify)
	{
		NotifyMemoryWrite(Memory::GetRegionOfAddress(address), address, value);
	}
}

void MMU::WriteWord(Memory::Address address, uint16_t value, bool notify)
{
	WriteByte(address, (value & 0xFF), notify);
	WriteByte(++address, ((value >> 8) & 0xFF), notify);
}

void MMU::LoadRom(const std::string &rom_file_path)
{
	rom_loaded_ = false;

	std::ifstream rom_read_stream{ rom_file_path, std::ios::binary | std::ios::ate };
	if (!rom_read_stream.is_open()) { throw std::runtime_error{ "ROM file could not be opened" }; }

	const auto file_size = static_cast<size_t>(rom_read_stream.tellg());
	rom_read_stream.seekg(0, std::ios::beg);

	rom_read_stream.read(reinterpret_cast<char*>(memory_.data()), std::max(file_size, memory_.size()));

	rom_read_stream.close();

	rom_loaded_ = true;
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
