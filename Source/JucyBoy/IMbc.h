#pragma once

#include <list>
#include <functional>
#include <vector>
#include <cstdint>
#include "Memory.h"

class MMU;

class IMbc
{
public:
	IMbc(MMU &mmu);
	virtual ~IMbc();

	virtual void OnRomWritten(Memory::Address address, uint8_t value) = 0;
	virtual std::vector<std::vector<uint8_t>> GetExternalRamBanks(uint8_t external_ram_size_code);

protected:
	MMU* mmu_;
	std::list<std::function<void()>> mmu_listener_deregister_functions_;
};
