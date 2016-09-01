#include "CPU.h"
#include "MMU.h"
#include <cassert>

CPU::CPU(MMU &mmu) :
	mmu_{ &mmu }
{
	PopulateInstructionNames();
	PopulateCbInstructionNames();
	PopulateInstructions();
	PopulateCbInstructions();
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

void CPU::Run()
{
	if (loop_function_result_.valid()) { return; }

	exit_loop_.store(false);
	loop_function_result_ = std::async(std::launch::async, &CPU::RunningLoopFunction, this);
}

void CPU::Stop()
{
	if (!loop_function_result_.valid()) { return; }

	exit_loop_.store(true);
	
	// Note: the following bug has been found in Visual Studio (including VS2015)
	//   According to C++ ˜30.6.6/16-17: Calling std::future::valid() after std::future::get() must always return false.
	//   However, if the future contained an exception, std::future::valid() will still return true after having called std::future::get().
	//   Therefore, CPU::IsRunning() will incorrectly return true after CPU::Stop() has been called if an exception had been thrown in the running loop.
	//   Ref: https://connect.microsoft.com/VisualStudio/feedback/details/1883150/c-std-future-get-does-not-release-shared-state-if-it-contains-an-exception-vc-2015
	//        http://stackoverflow.com/questions/33899615/stdfuture-still-valid-after-calling-get-which-throws-an-exception
	//
	// Workaround:
	//   Transfer the shared state out of the std::future into a std::shared_future object, and call get() in it instead.
	//   This way, successive calls to std::future::valid() will correctly return false.

	auto shared_future = loop_function_result_.share();
	assert(!loop_function_result_.valid());

	// get() will throw if any exception was thrown in the running loop
	shared_future.get();

	// Notify the state of the registers only when exitting the running loop, in order to keep a good performance
	NotifyCpuStateChange();
}

bool CPU::IsRunning() const noexcept
{
	return loop_function_result_.valid();
}

void CPU::RunningLoopFunction()
{
	try
	{
		//TODO: only check exit_loop_ once per frame (during VBlank), in order to increase performance
		while (!exit_loop_.load())
		{
			auto cycles = ExecuteInstruction(FetchOpcode());
		}
	}
	catch (std::exception &)
	{
		NotifyExceptionInRunningLoop();

		// Rethrow the exception that was just caught, in order to retrieve it later via future::get()
		throw;
	}
}

void CPU::StepOver()
{
	// The CPU must be used with either of these methods: 
	//   1) Calling Run to execute instructions until calling Stop
	//   2) Calling StepOver consecutively to execute one instruction at a time.
	// Therefore, do not allow calling StepOver if Run has already been called.
	if (IsRunning()) { throw std::logic_error{ "Trying to call StepOver while RunningLoopFunction thread is running" }; }

	auto cycles = ExecuteInstruction(FetchOpcode());
	NotifyCpuStateChange();
}

CPU::OpCode CPU::FetchOpcode()
{
	return mmu_->ReadByte(registers_.pc++);
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

void CPU::NotifyExceptionInRunningLoop() const
{
	for (auto& listener : listeners_)
	{
		listener->OnExceptionInRunningLoop();
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
