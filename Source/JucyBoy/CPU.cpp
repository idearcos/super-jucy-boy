#include "CPU.h"
#include "MMU.h"
#include <sstream>
#include <iomanip>

CPU::CPU(MMU &mmu) :
	mmu_{ &mmu }
{
	PopulateInstructions();
	Reset();
}

CPU::~CPU()
{
	Stop();
}

void CPU::Reset()
{
	if (IsRunning()) { throw std::logic_error{ "Trying to call Reset while RunningLoopFunction thread is running" }; }

	registers_.af.WriteWord(0x01B0);
	registers_.bc.WriteWord(0x0013);
	registers_.de.WriteWord(0x00D8);
	registers_.hl.WriteWord(0x014D);
	registers_.pc = 0x0100;
	registers_.sp = 0xFFFE;

	NotifyCpuStateChange();
}

void CPU::PopulateInstructions()
{
	for (size_t ii = 0; ii < instructions_.size(); ++ii)
	{
		instructions_[ii] = [this, ii]() -> MachineCycles {
			std::stringstream error;
			error << "Instruction not yet implemented for opcode: 0x"
				<< std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<size_t>(ii);
			throw std::runtime_error(error.str());
		};
	}
}

void CPU::Run()
{
	if (loop_function_thread_.joinable()) { return; }

	exit_loop_.store(false);
	loop_function_thread_ = std::thread{ &CPU::RunningLoopFunction, this };
}

void CPU::Stop()
{
	if (!loop_function_thread_.joinable()) { return; }

	exit_loop_.store(true);
	loop_function_thread_.join();
}

bool CPU::IsRunning() const
{
	return loop_function_thread_.joinable();
}

void CPU::RunningLoopFunction()
{
	//TODO: only check exit_loop_ once per frame (during VBlank), in order to increase performance
	while (!exit_loop_.load())
	{
		auto cycles = ExecuteInstruction(FetchOpcode());
	}

	// Notify the state of the registers only when exitting the loop, in order to respect performance
	NotifyCpuStateChange();
}

void CPU::StepOver()
{
	// The CPU must be used either by calling Run to execute instructions until calling Stop, OR by calling StepOver consecutively to execute one instruction at a time.
	// Therefore, do not allow calling StepOver while if Run has already been called.
	if (IsRunning()) { throw std::logic_error{ "Trying to call StepOver while RunningLoopFunction thread is running" }; }

	auto cycles = ExecuteInstruction(FetchOpcode());
	NotifyCpuStateChange();
}

CPU::OpCode CPU::FetchOpcode()
{
	return mmu_->Read(registers_.pc++);
}

CPU::MachineCycles CPU::ExecuteInstruction(OpCode opcode)
{
	return instructions_[opcode]();
}

#pragma region Listeners
void CPU::AddListener(Listener &listener)
{
	listeners_.insert(&listener);
}

void CPU::RemoveListener(Listener &listener)
{
	listeners_.erase(&listener);
}

void CPU::NotifyCpuStateChange() const
{
	for (auto& listener : listeners_)
	{
		listener->OnCpuStateChanged(registers_, ReadFlags());
	}
}
#pragma endregion

#pragma region Flag operations
void CPU::SetFlag(Flags flag)
{
	registers_.af.WriteLowByte(static_cast<uint8_t>(ReadFlags() | flag));
}

void CPU::ClearFlag(Flags flag)
{
	registers_.af.WriteLowByte(static_cast<uint8_t>(ReadFlags() & ~flag));
}

bool CPU::IsFlagSet(Flags flag) const
{
	return (static_cast<uint8_t>(ReadFlags() & flag)) != 0;
}

CPU::Flags CPU::ReadFlags() const
{
	return static_cast<Flags>(registers_.af.ReadLowByte());
}

void CPU::ClearAndSetFlags(Flags flags)
{
	registers_.af.WriteLowByte(static_cast<uint8_t>(flags));
}
#pragma endregion
