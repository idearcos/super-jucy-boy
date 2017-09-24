#include "MMU.h"

MMU::MMU()
{
	MapMemoryRead([this](Memory::Address address) { return OnUnusedMemoryRead(address); }, Memory::Region::ROM_Bank0);
	MapMemoryRead([this](Memory::Address address) { return OnUnusedMemoryRead(address); }, Memory::Region::ROM_OtherBanks);
	MapMemoryRead([this](Memory::Address address) { return OnUnusedMemoryRead(address); }, Memory::Region::ERAM);
	MapMemoryRead([this](Memory::Address address) { return OnWramRead(address); }, Memory::Region::WRAM);
	MapMemoryRead([this](Memory::Address address) { return OnWramEchoRead(address); }, Memory::Region::WRAM_Echo);
	MapMemoryRead([this](Memory::Address address) { return OnUnusedMemoryRead(address); }, Memory::Region::Unused);
	MapMemoryRead([this](Memory::Address address) { return OnIoMemoryRead(address); }, Memory::Region::IO);
	MapMemoryRead([this](Memory::Address address) { return OnHramRead(address); }, Memory::Region::HRAM);

	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnUnusedMemoryWritten(address, value); }, Memory::Region::ROM_Bank0);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnUnusedMemoryWritten(address, value); }, Memory::Region::ROM_OtherBanks);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnUnusedMemoryWritten(address, value); }, Memory::Region::ERAM);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnWramWritten(address, value); }, Memory::Region::WRAM);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnWramEchoWritten(address, value); }, Memory::Region::WRAM_Echo);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnUnusedMemoryWritten(address, value); }, Memory::Region::Unused);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnIoMemoryWritten(address, value); }, Memory::Region::IO);
	MapMemoryWrite([this](Memory::Address address, uint8_t value) { return OnHramWritten(address, value); }, Memory::Region::HRAM);

	mapped_io_register_reads_.fill([this](Memory::Address address) { return OnUnmappedIoRegisterRead(address); });
	mapped_io_register_writes_.fill([this](Memory::Address address, uint8_t value) { OnUnmappedIoRegisterWritten(address, value); });

	wram_.fill(0);
	hram_.fill(0);
	unmapped_io_registers_.fill(0xFF);
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
		mapped_io_register_reads_[address - Memory::io_offset_] = io_register_read_function;
	}
}

void MMU::MapIoRegisterWrite(MemoryWriteFunction &&io_register_write_function, Memory::Address first_register, Memory::Address last_register)
{
	for (Memory::Address address = first_register; address <= last_register; ++address)
	{
		mapped_io_register_writes_[address - Memory::io_offset_] = io_register_write_function;
	}
}
#pragma endregion

#pragma region Debug
Memory::Map MMU::GetMemoryMap() const
{
	Memory::Map memory_map{};

	for (int i = 0; i < memory_map.size(); ++i)
	{
		memory_map[i] = ReadByte(static_cast<uint16_t>(i));
	}

	return memory_map;
}
#pragma endregion
