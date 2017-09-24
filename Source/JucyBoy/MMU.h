#pragma once

#include <cstdint>
#include <array>
#include <functional>
#include "Memory.h"

class MMU
{
public:
	MMU();
	virtual ~MMU() = default;

	inline uint8_t ReadByte(Memory::Address address) const { return mapped_memory_reads_[static_cast<size_t>(Memory::GetRegion(address))](address); }
	void WriteByte(Memory::Address address, uint8_t value) { mapped_memory_writes_[static_cast<size_t>(Memory::GetRegion(address))](address, value); }

	inline void SetBit(Memory::Address address, int bit_num) { WriteByte(address, (1 << bit_num) | ReadByte(address)); }
	inline void ClearBit(Memory::Address address, int bit_num) { WriteByte(address, ~(1 << bit_num) & ReadByte(address)); }
	inline bool IsBitSet(Memory::Address address, int bit_num) { return (ReadByte(address) & (1 << bit_num)) != 0; }

	using MemoryReadFunction = std::function<uint8_t(Memory::Address address)>;
	using MemoryWriteFunction = std::function<void(Memory::Address address, uint8_t value)>;
	void MapMemoryRead(MemoryReadFunction &&memory_read_function, Memory::Region region);
	void MapMemoryWrite(MemoryWriteFunction &&memory_write_function, Memory::Region region);
	void MapIoRegisterRead(MemoryReadFunction &&io_register_read_function, Memory::Address first_register, Memory::Address last_register);
	void MapIoRegisterWrite(MemoryWriteFunction &&io_register_write_function, Memory::Address first_register, Memory::Address last_register);

	// Debug / GUI interaction
	Memory::Map GetMemoryMap() const;

	template<class Archive>
	void serialize(Archive &archive);

private:
	uint8_t OnWramRead(Memory::Address address) const { return wram_[address - Memory::wram_offset_]; }
	void OnWramWritten(Memory::Address address, uint8_t value) { wram_[address - Memory::wram_offset_] = value; }

	uint8_t OnWramEchoRead(Memory::Address address) const { return wram_[address - Memory::wram_echo_offset_]; }
	void OnWramEchoWritten(Memory::Address address, uint8_t value) { wram_[address - Memory::wram_echo_offset_] = value; }

	uint8_t OnUnusedMemoryRead(const Memory::Address&) const { return 0xFF; }
	void OnUnusedMemoryWritten(const Memory::Address&, uint8_t) { return; }

	uint8_t OnIoMemoryRead(Memory::Address address) const { return mapped_io_register_reads_[address - Memory::io_offset_](address); }
	void OnIoMemoryWritten(Memory::Address address, uint8_t value) { mapped_io_register_writes_[address - Memory::io_offset_](address, value); }

	uint8_t OnUnmappedIoRegisterRead(Memory::Address address) const { return unmapped_io_registers_[address - Memory::io_offset_]; }
	void OnUnmappedIoRegisterWritten(Memory::Address address, uint8_t value) { unmapped_io_registers_[address - Memory::io_offset_] = value; }

	uint8_t OnHramRead(Memory::Address address) const { return hram_[address - Memory::hram_offset_]; }
	void OnHramWritten(Memory::Address address, uint8_t value) { hram_[address - Memory::hram_offset_] = value; }

private:
	std::array<MemoryReadFunction, static_cast<size_t>(Memory::Region::Count)> mapped_memory_reads_;
	std::array<MemoryWriteFunction, static_cast<size_t>(Memory::Region::Count)> mapped_memory_writes_;
	std::array<MemoryReadFunction, Memory::io_region_size_> mapped_io_register_reads_;
	std::array<MemoryWriteFunction, Memory::io_region_size_> mapped_io_register_writes_;

	std::array<uint8_t, Memory::wram_size_> wram_;
	std::array<uint8_t, Memory::hram_size_> hram_;

	std::array<uint8_t, Memory::io_region_size_> unmapped_io_registers_;

private:
	MMU(const MMU&) = delete;
	MMU(MMU&&) = delete;
	MMU& operator=(const MMU&) = delete;
	MMU& operator=(MMU&&) = delete;
};

template<class Archive>
void MMU::serialize(Archive &archive)
{
	archive(wram_, hram_);
}
