#include "DebugCPU.h"
#include "DebugMMU.h"

DebugCPU::DebugCPU(DebugMMU &debug_mmu) : CPU{ debug_mmu },
	debug_mmu_{ &debug_mmu }
{

}

void DebugCPU::Run()
{
	if (loop_function_result_.valid()) { return; }

	exit_loop_.store(false);
	loop_function_result_ = std::async(std::launch::async, &DebugCPU::DebugRunningLoopFunction, this);
}

void DebugCPU::StepOver()
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

			if (IsBreakpointHit() || IsInstructionBreakpointHit() || IsWatchpointHit(debug_mmu_->ReadByte(registers_.pc)))
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

void DebugCPU::AddBreakpoint(Memory::Address address)
{
	//TODO: allow only when not running!
	breakpoints_.insert(address);
	NotifyBreakpointsChange();
}

void DebugCPU::RemoveBreakpoint(Memory::Address address)
{
	//TODO: allow only when not running!
	breakpoints_.erase(address);
	NotifyBreakpointsChange();
}

void DebugCPU::AddInstructionBreakpoint(OpCode opcode)
{
	instruction_breakpoints_.insert(opcode);
	NotifyInstructionBreakpointsChange();
}

void DebugCPU::RemoveInstructionBreakpoint(OpCode opcode)
{
	instruction_breakpoints_.erase(opcode);
	NotifyInstructionBreakpointsChange();
}

bool DebugCPU::IsBreakpointHit() const
{
	for (const auto breakpoint : breakpoints_)
	{
		if (breakpoint == registers_.pc)
		{
			return true;
		}
	}

	return false;
}

bool DebugCPU::IsInstructionBreakpointHit() const
{
	for (const auto instruction_breakpoint : instruction_breakpoints_)
	{
		if (instruction_breakpoint == debug_mmu_->ReadByte(registers_.pc))
		{
			return true;
		}
	}

	return false;
}

bool DebugCPU::IsWatchpointHit(OpCode next_opcode) const
{
	//TODO: also check OAM DMA for watchpoints
	Memory::Address address{ 0 };
	switch (next_opcode)
	{
	case 0x02:
		return debug_mmu_->IsWriteWatchpointHit(registers_.bc);
	case 0x08:
		address = debug_mmu_->ReadByte(registers_.pc + 1);
		address += (debug_mmu_->ReadByte(registers_.pc + 2) << 8);
		return debug_mmu_->IsWriteWatchpointHit(address);
	case 0x0A:
		return debug_mmu_->IsReadWatchpointHit(registers_.bc);
	case 0x12:
		return debug_mmu_->IsWriteWatchpointHit(registers_.de);
	case 0x1A:
		return debug_mmu_->IsReadWatchpointHit(registers_.de);
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
		return debug_mmu_->IsWriteWatchpointHit(registers_.hl);
	case 0x34:
	case 0x35:
		return debug_mmu_->IsReadWatchpointHit(registers_.hl) || debug_mmu_->IsWriteWatchpointHit(registers_.hl);
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
		return debug_mmu_->IsReadWatchpointHit(registers_.hl);
	case 0xC1:
	case 0xD1:
	case 0xE1:
	case 0xF1:
		return debug_mmu_->IsReadWatchpointHit(registers_.sp);
	case 0xC5:
	case 0xD5:
	case 0xE5:
	case 0xF5:
		return debug_mmu_->IsWriteWatchpointHit(registers_.sp);
	case 0xE0:
		return debug_mmu_->IsWriteWatchpointHit(Memory::io_region_start_ + debug_mmu_->ReadByte(registers_.pc + 1));
	case 0xE2:
		return debug_mmu_->IsWriteWatchpointHit(Memory::io_region_start_ + registers_.bc.GetLow());
	case 0xEA:
		address = debug_mmu_->ReadByte(registers_.pc + 1);
		address += (debug_mmu_->ReadByte(registers_.pc + 2) << 8);
		return debug_mmu_->IsWriteWatchpointHit(address);
	case 0xF0:
		return debug_mmu_->IsReadWatchpointHit(Memory::io_region_start_ + debug_mmu_->ReadByte(registers_.pc + 1));
	case 0xF2:
		return debug_mmu_->IsReadWatchpointHit(Memory::io_region_start_ + registers_.bc.GetLow());
	case 0xFA:
		address = debug_mmu_->ReadByte(registers_.pc + 1);
		address += (debug_mmu_->ReadByte(registers_.pc + 2) << 8);
		return debug_mmu_->IsReadWatchpointHit(address);
	default:
		return false;
	}
}

#pragma region Listener notification
void DebugCPU::NotifyBreakpointsChange() const
{
	for (auto& listener : listeners_)
	{
		listener->OnBreakpointsChanged(breakpoints_);
	}
}

void DebugCPU::NotifyInstructionBreakpointsChange() const
{
	for (auto& listener : listeners_)
	{
		listener->OnInstructionBreakpointsChanged(instruction_breakpoints_);
	}
}
#pragma endregion

