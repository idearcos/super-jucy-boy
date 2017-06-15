#pragma once

#include "../MMU.h"
#include <set>

class DebugMMU final : public MMU
{
public:
	DebugMMU() = default;
	~DebugMMU() = default;

	bool IsReadWatchpointHit(Memory::Address address) const;
	bool IsWriteWatchpointHit(Memory::Address address) const;

	// GUI interaction
	Memory::Map GetMemoryMap() const;
	std::vector<Memory::Watchpoint> GetWatchpointList() const;
	void AddWatchpoint(Memory::Watchpoint watchpoint);
	void RemoveWatchpoint(Memory::Watchpoint watchpoint);

private:
	// Watchpoints
	std::set<Memory::Address> read_watchpoints_;
	std::set<Memory::Address> write_watchpoints_;

private:
	DebugMMU(const DebugMMU&) = delete;
	DebugMMU(DebugMMU&&) = delete;
	DebugMMU& operator=(const DebugMMU&) = delete;
	DebugMMU& operator=(DebugMMU&&) = delete;
};
