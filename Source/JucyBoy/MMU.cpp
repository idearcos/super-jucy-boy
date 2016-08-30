#include "MMU.h"
#include <fstream>

MMU::MMU() :
	memory_{} // Value-initialize to all-zeroes
{

}

MMU::~MMU()
{

}

void MMU::Reset()
{
	memory_[0xFF05] = 0x00; //TIMA
	memory_[0xFF06] = 0x00; //TMA
	memory_[0xFF07] = 0x00; //TAC
	memory_[0xFF10] = 0x80; //NR10
	memory_[0xFF11] = 0xBF; //NR11
	memory_[0xFF12] = 0xF3; //NR12
	memory_[0xFF14] = 0xBF; //NR14
	memory_[0xFF16] = 0x3F; //NR21
	memory_[0xFF17] = 0x00; //NR22
	memory_[0xFF19] = 0xBF; //NR24
	memory_[0xFF1A] = 0x7F; //NR30
	memory_[0xFF1B] = 0xFF; //NR31
	memory_[0xFF1C] = 0x9F; //NR32
	memory_[0xFF1E] = 0xBF; //NR33
	memory_[0xFF20] = 0xFF; //NR41
	memory_[0xFF21] = 0x00; //NR42
	memory_[0xFF22] = 0x00; //NR43
	memory_[0xFF23] = 0xBF; //NR30
	memory_[0xFF24] = 0x77; //NR50
	memory_[0xFF25] = 0xF3; //NR51
	memory_[0xFF26] = 0xF1; //NR52
	memory_[0xFF40] = 0x91; //LCDC
	memory_[0xFF42] = 0x00; //SCY
	memory_[0xFF43] = 0x00; //SCX
	memory_[0xFF45] = 0x00; //LYC
	memory_[0xFF47] = 0xFC; //BGP
	memory_[0xFF48] = 0xFF; //OBP0
	memory_[0xFF49] = 0xFF; //OBP1
	memory_[0xFF4A] = 0x00; //WY
	memory_[0xFF4B] = 0x00; //WX
	memory_[0xFFFF] = 0x00; //IE
}

uint8_t MMU::Read(Address address) const
{
	return memory_[address];
}

void MMU::Write(Address address, uint8_t value)
{
	memory_[address] = value;
}

void MMU::LoadRom(const std::string &rom_file_path)
{
	std::ifstream rom_read_stream{ rom_file_path, std::ios::binary | std::ios::ate };
	if (!rom_read_stream.is_open()) { throw std::runtime_error{ "ROM file could not be opened" }; }

	const auto file_size = static_cast<size_t>(rom_read_stream.tellg());
	rom_read_stream.seekg(0, std::ios::beg);

	rom_read_stream.read(reinterpret_cast<char*>(memory_.data()), std::max(file_size, memory_.size()));

	rom_read_stream.close();
}
