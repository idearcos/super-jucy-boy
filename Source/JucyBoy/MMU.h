#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <list>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include "Memory.h"
#include "IMbc.h"

class MMU
{
public:
	using Listener = std::function<void(Memory::Address address, uint8_t value)>;

	MMU();
	virtual ~MMU();

	// Sets certain memory registers' initial state
	void Reset();

	uint8_t ReadByte(Memory::Address address) const;
	void WriteByte(Memory::Address address, uint8_t value, bool notify = true);

	inline void SetBit(Memory::Address address, int bit_num, bool notify = true) { WriteByte(address, (1 << bit_num) | ReadByte(address), notify); }
	inline void ClearBit(Memory::Address address, int bit_num, bool notify = true) { WriteByte(address, ~(1 << bit_num) & ReadByte(address), notify); }
	inline bool IsBitSet(Memory::Address address, int bit_num) { return (ReadByte(address) & (1 << bit_num)) != 0; }

	void LoadRom(const std::string &rom_file_path);
	bool IsRomLoaded() const noexcept { return rom_loaded_; }

	// Functions called by IMbc
	void EnableExternalRam(bool enable) { external_ram_enabled_ = enable; }
	void LoadRomBank(size_t rom_bank_number);
	void LoadRamBank(size_t ram_bank_number);

	// Functions called by OamDma
	bool IsOamDmaActive() const { return is_oam_dma_active_; }
	void OamDmaActive(bool is_active) { is_oam_dma_active_ = is_active; }

	// AddListener returns a deregister function that can be called with no arguments
	std::function<void()> AddListener(Listener &&listener_function, Memory::Region region)
	{
		auto it = listeners_[region].emplace(listeners_[region].begin(), listener_function);
		return [this, region, it]() { listeners_[region].erase(it); };
	}

private:
	// Listener notification
	void NotifyMemoryWrite(Memory::Region region, Memory::Address address, uint8_t value);

protected:
	std::vector<std::vector<uint8_t>> memory_;

private:
	std::vector<std::vector<uint8_t>> rom_banks;
	std::vector<std::vector<uint8_t>> external_ram_banks;

	std::unique_ptr<IMbc> mbc_;

	bool rom_loaded_{ false };
	size_t loaded_rom_bank_{ 1 };
	size_t loaded_external_ram_bank_{ 0 };
	bool external_ram_enabled_{ false };

	bool is_oam_dma_active_{ false };

	std::map<Memory::Region, std::list<Listener>> listeners_;

private:
	MMU(const MMU&) = delete;
	MMU(MMU&&) = delete;
	MMU& operator=(const MMU&) = delete;
	MMU& operator=(MMU&&) = delete;
};
