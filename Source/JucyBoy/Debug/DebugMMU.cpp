#include "DebugMMU.h"

bool DebugMMU::IsReadWatchpointHit(Memory::Address address) const
{
	return read_watchpoints_.count(address) != 0;
}

bool DebugMMU::IsWriteWatchpointHit(Memory::Address address) const
{
	return write_watchpoints_.count(address) != 0;
}

Memory::Map DebugMMU::GetMemoryMap() const
{
	Memory::Map memory_map{};

	for (int i = 0; i < memory_map.size(); ++i)
	{
		memory_map[i] = ReadByte(static_cast<Memory::Address>(i));
	}

	return memory_map;
}

std::vector<Memory::Watchpoint> DebugMMU::GetWatchpointList() const
{
	std::vector<Memory::Watchpoint> watchpoints;
	for (auto watchpoint_address : read_watchpoints_)
	{
		watchpoints.emplace_back(watchpoint_address, Memory::Watchpoint::Type::Read);
	}
	for (auto watchpoint_address : write_watchpoints_)
	{
		watchpoints.emplace_back(watchpoint_address, Memory::Watchpoint::Type::Write);
	}
	return watchpoints;
}

void DebugMMU::AddWatchpoint(Memory::Watchpoint watchpoint)
{
	switch (watchpoint.type)
	{
	case Memory::Watchpoint::Type::Read:
		read_watchpoints_.emplace(watchpoint.address);
		break;
	case Memory::Watchpoint::Type::Write:
		write_watchpoints_.emplace(watchpoint.address);
		break;
	default:
		break;
	}
}

void DebugMMU::RemoveWatchpoint(Memory::Watchpoint watchpoint)
{
	switch (watchpoint.type)
	{
	case Memory::Watchpoint::Type::Read:
		read_watchpoints_.erase(watchpoint.address);
		break;
	case Memory::Watchpoint::Type::Write:
		write_watchpoints_.erase(watchpoint.address);
		break;
	default:
		break;
	}
}
