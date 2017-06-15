#include "MMU.h"

MMU::MMU()
{
	MapMemoryRead(std::bind(&MMU::OnUnusedMemoryRead, this, std::placeholders::_1), Memory::Region::ROM_Bank0);
	MapMemoryRead(std::bind(&MMU::OnUnusedMemoryRead, this, std::placeholders::_1), Memory::Region::ROM_OtherBanks);
	MapMemoryRead(std::bind(&MMU::OnUnusedMemoryRead, this, std::placeholders::_1), Memory::Region::ERAM);
	MapMemoryRead(std::bind(&MMU::OnWramRead, this, std::placeholders::_1), Memory::Region::WRAM);
	MapMemoryRead(std::bind(&MMU::OnWramRead, this, std::placeholders::_1), Memory::Region::WRAM_Echo);
	MapMemoryRead(std::bind(&MMU::OnUnusedMemoryRead, this, std::placeholders::_1), Memory::Region::Unused);
	MapMemoryRead(std::bind(&MMU::OnIoMemoryRead, this, std::placeholders::_1), Memory::Region::IO);
	MapMemoryRead(std::bind(&MMU::OnHramRead, this, std::placeholders::_1), Memory::Region::HRAM);

	MapMemoryWrite(std::bind(&MMU::OnUnusedMemoryWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::ROM_Bank0);
	MapMemoryWrite(std::bind(&MMU::OnUnusedMemoryWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::ROM_OtherBanks);
	MapMemoryWrite(std::bind(&MMU::OnUnusedMemoryWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::ERAM);
	MapMemoryWrite(std::bind(&MMU::OnWramWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::WRAM);
	MapMemoryWrite(std::bind(&MMU::OnWramWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::WRAM_Echo);
	MapMemoryWrite(std::bind(&MMU::OnUnusedMemoryWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::Unused);
	MapMemoryWrite(std::bind(&MMU::OnIoMemoryWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::IO);
	MapMemoryWrite(std::bind(&MMU::OnHramWritten, this, std::placeholders::_1, std::placeholders::_2), Memory::Region::HRAM);

	mapped_io_register_reads_.fill(std::bind(&MMU::OnUnmappedIoRegisterRead, this, std::placeholders::_1));
	mapped_io_register_writes_.fill(std::bind(&MMU::OnUnmappedIoRegisterWritten, this, std::placeholders::_1, std::placeholders::_2));

	wram_.fill(0);
	hram_.fill(0);
	unmapped_io_registers_.fill(0xFF);
}

void MMU::Reset()
{
	wram_.fill(0);
	hram_.fill(0);
	unmapped_io_registers_.fill(0xFF);

	WriteByte(Memory::JOYP, 0xCF);
	WriteByte(Memory::TIMA, 0x00);
	WriteByte(Memory::TMA, 0x00);
	WriteByte(Memory::TAC, 0x00);
	WriteByte(Memory::NR10, 0x80);
	WriteByte(Memory::NR11, 0xBF);
	WriteByte(Memory::NR12, 0xF3);
	WriteByte(Memory::NR14, 0xBF);
	WriteByte(Memory::NR21, 0x3F);
	WriteByte(Memory::NR22, 0x00);
	WriteByte(Memory::NR24, 0xBF);
	WriteByte(Memory::NR30, 0x7F);
	WriteByte(Memory::NR31, 0xFF);
	WriteByte(Memory::NR32, 0x9F);
	WriteByte(Memory::NR33, 0xBF);
	WriteByte(Memory::NR41, 0xFF);
	WriteByte(Memory::NR42, 0x00);
	WriteByte(Memory::NR43, 0x00);
	WriteByte(Memory::NR44, 0xBF);
	WriteByte(Memory::NR50, 0x77);
	WriteByte(Memory::NR51, 0xF3);
	WriteByte(Memory::NR52, 0xF1);
	WriteByte(Memory::LCDC, 0x91);
	WriteByte(Memory::SCY, 0x00);
	WriteByte(Memory::SCX, 0x00);
	WriteByte(Memory::LYC, 0x00);
	WriteByte(Memory::BGP, 0xFC);
	WriteByte(Memory::OBP0, 0xFF);
	WriteByte(Memory::OBP1, 0xFF);
	WriteByte(Memory::WY, 0x00);
	WriteByte(Memory::WX, 0x00);
	WriteByte(Memory::IE, 0x00);
}

uint8_t MMU::ReadByte(Memory::Address address) const
{
	const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

	return mapped_memory_reads_[static_cast<size_t>(region_and_relative_address.first)](region_and_relative_address.second);
}

void MMU::WriteByte(Memory::Address address, uint8_t value)
{
	const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

	mapped_memory_writes_[static_cast<size_t>(region_and_relative_address.first)](region_and_relative_address.second, value);
}

#pragma region Memory read/write function mapping
void MMU::MapMemoryRead(MemoryReadFunction &&memory_read_function, Memory::Region region)
{
	mapped_memory_reads_[static_cast<size_t>(region)] = memory_read_function;
}

void MMU::MapMemoryWrite(MemoryWriteFunction &&memory_write_function, Memory::Region region)
{
	mapped_memory_writes_[static_cast<size_t>(region)] = memory_write_function;
}

void MMU::MapIoRegisterRead(MemoryReadFunction &&io_register_read_function, Memory::Address first_register, Memory::Address last_register)
{
	for (Memory::Address address = first_register; address <= last_register; ++address)
	{
		const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

		if (region_and_relative_address.first != Memory::Region::IO) throw std::logic_error{ "Trying to map IO register read function to non-IO memory address" };

		mapped_io_register_reads_[region_and_relative_address.second] = io_register_read_function;
	}
}

void MMU::MapIoRegisterWrite(MemoryWriteFunction &&io_register_write_function, Memory::Address first_register, Memory::Address last_register)
{
	for (Memory::Address address = first_register; address <= last_register; ++address)
	{
		const auto region_and_relative_address = Memory::GetRegionAndRelativeAddress(address);

		if (region_and_relative_address.first != Memory::Region::IO) throw std::logic_error{ "Trying to map IO register write function to non-IO memory address" };

		mapped_io_register_writes_[region_and_relative_address.second] = io_register_write_function;
	}
}
#pragma endregion
