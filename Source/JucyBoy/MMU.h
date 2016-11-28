#pragma once

#include <cstdint>
#include <array>
#include <limits>
#include <string>
#include <list>
#include <map>
#include <functional>
#include "Memory.h"

class MMU
{
public:
	MMU();
	~MMU();

	// Sets certain memory registers' initial state
	void Reset();

	uint8_t ReadByte(Memory::Address address) const;
	uint16_t ReadWord(Memory::Address address) const;
	void WriteByte(Memory::Address address, uint8_t value, bool notify = true);
	void WriteWord(Memory::Address address, uint16_t value, bool notify = true);

	template <int BitNum>
	void SetBit(Memory::Address address, bool notify = true) { WriteByte(address, (1 << BitNum) | ReadByte(address), notify); }
	template <int BitNum>
	void ClearBit(Memory::Address address, bool notify = true) { WriteByte(address, ~(1 << BitNum) & ReadByte(address), notify); }
	template <int BitNum>
	bool IsBitSet(Memory::Address address) { return (ReadByte(address) & (1 << BitNum)) != 0; }

	void LoadRom(const std::string &rom_file_path);
	bool IsRomLoaded() const noexcept { return rom_loaded_; }

	using MemoryMap = std::array<uint8_t, std::numeric_limits<Memory::Address>::max() + 1>;
	MemoryMap GetMemoryMap() const { return memory_; }

	// AddListener returns a deregister function that can be called with no arguments
	template <typename T>
	std::function<void()> AddListener(T &listener, void(T::*func)(Memory::Address, uint8_t), Memory::Region region)
	{
		auto it = listeners_[region].emplace(listeners_[region].begin(), std::bind(func, std::ref(listener), std::placeholders::_1, std::placeholders::_2));
		return [=, this](){ listeners_[region].erase(it); };
	}

private:
	// Listener notification
	void NotifyMemoryWrite(Memory::Region region, Memory::Address address, uint8_t value);

private:
	MemoryMap memory_{}; // Value-initialize to all-zeroes

	bool rom_loaded_{ false };

	using Listener = std::function<void(Memory::Address address, uint8_t value)>;
	std::map<Memory::Region, std::list<Listener>> listeners_;
};
