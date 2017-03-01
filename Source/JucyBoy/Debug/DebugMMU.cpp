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
	size_t offset{ 0 };

	for (int i = 0; i < memory_.size(); ++i)
	{
		memcpy(memory_map.data() + offset, memory_[i].data(), memory_[i].size());

		// memory_[i].size() cannot be used below, since in the MBC2 case the external RAM size will be 2kBytes, rather than the usual 8 kBytes
		offset += GetSizeOfRegion(static_cast<Memory::Region>(i));
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
