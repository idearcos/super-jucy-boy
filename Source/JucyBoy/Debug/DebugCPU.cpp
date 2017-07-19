#include "DebugCPU.h"
#include "../MMU.h"

DebugCPU::DebugCPU(MMU &mmu) : CPU{ mmu }
{

}

void DebugCPU::DebugRun()
{
	if (loop_function_result_.valid()) { return; }

	exit_loop_.store(false);
	loop_function_result_ = std::async(std::launch::async, &DebugCPU::DebugRunningLoopFunction, this);
}

void DebugCPU::DebugStepOver()
{
	// The CPU must be used with either of these methods:
	//   1) Calling Run to execute instructions until calling Stop
	//   2) Calling StepOver consecutively to execute one instruction at a time.
	// Therefore, do not allow calling StepOver if Run has already been called.
	if (IsRunning()) { throw std::logic_error{ "Trying to call StepOver while RunningLoopFunction thread is running" }; }

	ExecuteOneInstruction();

	//TODO: Notify hit breakpoints/watchpoints
}

void DebugCPU::DebugRunningLoopFunction()
{
	try
	{
		//TODO: only check exit_loop_ once per frame (during VBlank), in order to increase performance
		//TODO: precompute the next breakpoint instead of iterating the whole set every time. This "next breakpoint" would need to be updated after every Jump instruction.
		while (!exit_loop_.load())
		{
			ExecuteOneInstruction();

			if (IsBreakpointHit() || IsInstructionBreakpointHit() || IsWatchpointHit(mmu_->ReadByte(registers_.pc)))
			{
				NotifyRunningLoopInterruption();
				break;
			}
		}
	}
	catch (std::exception &)
	{
		NotifyRunningLoopInterruption();

		// Rethrow the exception that was just caught, in order to retrieve it later via future::get()
		throw;
	}
}

bool DebugCPU::IsBreakpointHit() const
{
	const auto breakpoint_hit = (breakpoints_.find(registers_.pc) != breakpoints_.end());
	if (breakpoint_hit) NotifyBreakpointHit(registers_.pc);

	return breakpoint_hit;
}

bool DebugCPU::IsInstructionBreakpointHit() const
{
	const auto instruction_breakpoint_hit = (instruction_breakpoints_.find(mmu_->ReadByte(registers_.pc)) != instruction_breakpoints_.end());
	if (instruction_breakpoint_hit) NotifyInstructionBreakpointHit(mmu_->ReadByte(registers_.pc));

	return instruction_breakpoint_hit;
}

bool DebugCPU::IsWatchpointHit(OpCode next_opcode) const
{
	//TODO: also check OAM DMA for watchpoints
	//TODO: check Call from interrupts
	Memory::Address address{ 0 };
	switch (next_opcode)
	{
	case 0x02:
		return IsWriteWatchpointHit(registers_.bc);
	case 0x08:
		address = mmu_->ReadByte(registers_.pc + 1);
		address += (mmu_->ReadByte(registers_.pc + 2) << 8);
		return IsWriteWatchpointHit(address) || IsWriteWatchpointHit(address + 1);
	case 0x0A:
		return IsReadWatchpointHit(registers_.bc);
	case 0x12:
		return IsWriteWatchpointHit(registers_.de);
	case 0x1A:
		return IsReadWatchpointHit(registers_.de);
	case 0x22:
	case 0x32:
	case 0x36:
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x77:
		return IsWriteWatchpointHit(registers_.hl);
	case 0x34:
	case 0x35:
		return IsReadWatchpointHit(registers_.hl) || IsWriteWatchpointHit(registers_.hl);
	case 0x2A:
	case 0x3A:
	case 0x46:
	case 0x4E:
	case 0x56:
	case 0x5E:
	case 0x66:
	case 0x6E:
	case 0x7E:
	case 0x86:
	case 0x8E:
	case 0x96:
	case 0x9E:
	case 0xA6:
	case 0xAE:
	case 0xB6:
	case 0xBE:
		return IsReadWatchpointHit(registers_.hl);
	case 0xC1:
	case 0xD1:
	case 0xE1:
	case 0xF1:
		return IsReadWatchpointHit(registers_.sp) || IsReadWatchpointHit(registers_.sp + 1);
	case 0xC5:
	case 0xD5:
	case 0xE5:
	case 0xF5:
	case 0xC7:
	case 0xCF:
	case 0xD7:
	case 0xDF:
	case 0xE7:
	case 0xEF:
	case 0xF7:
	case 0xFF:
	case 0xC4:
	case 0xCC:
	case 0xCD:
	case 0xD4:
	case 0xDC:
		return IsWriteWatchpointHit(registers_.sp - 1) || IsWriteWatchpointHit(registers_.sp - 2);
	case 0xE0:
		return IsWriteWatchpointHit(Memory::io_region_start_ + mmu_->ReadByte(registers_.pc + 1));
	case 0xE2:
		return IsWriteWatchpointHit(Memory::io_region_start_ + registers_.bc.GetLow());
	case 0xEA:
		address = mmu_->ReadByte(registers_.pc + 1);
		address += (mmu_->ReadByte(registers_.pc + 2) << 8);
		return IsWriteWatchpointHit(address);
	case 0xF0:
		return IsReadWatchpointHit(Memory::io_region_start_ + mmu_->ReadByte(registers_.pc + 1));
	case 0xF2:
		return IsReadWatchpointHit(Memory::io_region_start_ + registers_.bc.GetLow());
	case 0xFA:
		address = mmu_->ReadByte(registers_.pc + 1);
		address += (mmu_->ReadByte(registers_.pc + 2) << 8);
		return IsReadWatchpointHit(address);
	default:
		return false;
	}
}

bool DebugCPU::IsReadWatchpointHit(Memory::Address address) const
{
	const auto watchpoint_hit = (read_watchpoints_.count(address) != 0);
	if (watchpoint_hit) NotifyWatchpointHit({ address, Memory::Watchpoint::Type::Read });

	return watchpoint_hit;
}

bool DebugCPU::IsWriteWatchpointHit(Memory::Address address) const
{
	const auto watchpoint_hit = (write_watchpoints_.count(address) != 0);
	if (watchpoint_hit) NotifyWatchpointHit({ address, Memory::Watchpoint::Type::Write });

	return watchpoint_hit;
}

std::vector<Memory::Watchpoint> DebugCPU::GetWatchpointList() const
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

void DebugCPU::AddWatchpoint(Memory::Watchpoint watchpoint)
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

void DebugCPU::RemoveWatchpoint(Memory::Watchpoint watchpoint)
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

#pragma region Listener notification
void DebugCPU::NotifyBreakpointHit(Memory::Address breakpoint) const
{
	for (const auto &listener : listeners_)
	{
		listener->OnBreakpointHit(breakpoint);
	}
}

void DebugCPU::NotifyInstructionBreakpointHit(OpCode opcode) const
{
	for (const auto &listener : listeners_)
	{
		listener->OnInstructionBreakpointHit(opcode);
	}
}

void DebugCPU::NotifyWatchpointHit(Memory::Watchpoint watchpoint) const
{
	for (const auto &listener : listeners_)
	{
		listener->OnWatchpointHit(watchpoint);
	}
}
#pragma endregion
