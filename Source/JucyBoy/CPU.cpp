#include "CPU.h"
#include "MMU.h"
#include <cassert>
#include <string>

CPU::CPU(MMU &mmu) :
	mmu_{ &mmu }
{
	PopulateInstructions();
	PopulateCbInstructions();
}

CPU::~CPU()
{
	Stop();
}

void CPU::Reset()
{
	if (IsRunning()) { throw std::logic_error{ "Trying to call Reset while RunningLoopFunction thread is running" }; }

	registers_.af = 0x01B0;
	registers_.bc = 0x0013;
	registers_.de = 0x00D8;
	registers_.hl = 0x014D;
	registers_.pc = 0x0100;
	registers_.sp = 0xFFFE;
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
		//TODO: precompute the next breakpoint instead of iterating the whole set every time. This "next breakpoint" would need to be updated after every Jump instruction.
		while (!exit_loop_.load())
		{
			ExecuteOneInstruction();
		}
	}
	catch (std::exception &)
	{
		NotifyRunningLoopInterruption();

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

	ExecuteInstruction(FetchOpcode());

	CheckInterrupts();
}

void CPU::ExecuteOneInstruction()
{
	switch (current_state_)
	{
	case State::Running:
		previous_pc_ = registers_.pc;
		ExecuteInstruction(FetchOpcode());
		break;
	case State::Halted:
		// NOP instructions are executed until Halted state ends
		NotifyMachineCycleLapse();
		break;
	case State::HaltBug:
		// PC increment is skipped once when fetching the next opcode (no extra cycles are lapsed)
		current_state_ = State::Running;
		previous_pc_ = registers_.pc;
		ExecuteInstruction(mmu_->ReadByte(registers_.pc));
		break;
	case State::Stopped:
		//TODO check for joypad input, since that is the only thing that can finish Stopped state
		return;
	}

	CheckInterrupts();
}

#pragma region Memory R/W
uint8_t CPU::FetchByte()
{
	return ReadByte(registers_.pc++);
}

uint16_t CPU::FetchWord()
{
	uint16_t value{ FetchByte() };
	value += (FetchByte() << 8);
	return value;
}

uint16_t CPU::PopWordFromStack()
{
	uint16_t value{ ReadByte(registers_.sp++) };
	value += (ReadByte(registers_.sp++) << 8);
	return value;
}

void CPU::PushWordToStack(uint16_t value)
{
	NotifyMachineCycleLapse();
	WriteByte(--registers_.sp, (value >> 8) & 0xFF);
	WriteByte(--registers_.sp, value & 0xFF);
}

uint8_t CPU::ReadByte(uint16_t address) const
{
	const auto value = mmu_->ReadByte(address);
	NotifyMachineCycleLapse();
	return value;
}

void CPU::WriteByte(uint16_t address, uint8_t value) const
{
	mmu_->WriteByte(address, value);
	NotifyMachineCycleLapse();
}
#pragma endregion

#pragma region Interrupts
void CPU::CheckInterrupts()
{
	switch (current_state_)
	{
	case State::Running:
		if (!interrupt_master_enable_)
		{
			break;
		}
	case State::Halted:
		for (auto interrupt = static_cast<uint16_t>(Interrupt::VBlank); interrupt <= static_cast<uint16_t>(Interrupt::Joypad); ++interrupt)
		{
			if ((enabled_interrupts_ & (1 << interrupt)) && (requested_interrupts_ & (1 << interrupt)))
			{
				current_state_ = State::Running;

				if (!interrupt_master_enable_) { break; }

				// There are 2 additional machine cycles spent here
				NotifyMachineCycleLapse();
				NotifyMachineCycleLapse();

				// Call appropriate Interrupt Service Routine
				Call(Memory::isr_start_ + (interrupt * 0x08));

				// Interrupt Master Enable and the corresponding bit in the IF register become cleared
				interrupt_master_enable_ = false;
				requested_interrupts_ &= (~(1 << interrupt));

				// Interrupts are processed one at a time, therefore exit the loop now
				break;
			}
		}
		break;
	case State::Stopped:
		break;
	case State::HaltBug:
		// HaltBug is only reached when IME is disabled, therefore interrupt servicing is not needed
		// (The state will change to Running in the next CPU step)
		break;
	default:
		throw std::runtime_error{ "Invalid CPU state in interrupt check: " + std::to_string(static_cast<int>(current_state_)) };
	}

	// EI enables the interrupts for the instruction AFTER itself
	if (ime_requested_)
	{
		interrupt_master_enable_ = true;
		ime_requested_ = false;
	}
}
#pragma endregion

#pragma region Instruction helper functions
void CPU::IncrementRegister(uint8_t &reg)
{
	ClearFlag(Flags::Z | Flags::H | Flags::N);
	const auto result = ++reg;
	if ((result & 0x0F) == 0x00) { SetFlag(Flags::H); }
	if (result == 0) { SetFlag(Flags::Z); }
}

void CPU::DecrementRegister(uint8_t &reg)
{
	ClearFlag(Flags::Z | Flags::H);
	SetFlag(Flags::N);
	const auto result = --reg;
	if ((result & 0x0F) == 0x0F) { SetFlag(Flags::H); }
	if (result == 0) { SetFlag(Flags::Z); }
}

void CPU::Add(uint8_t value)
{
	ClearFlag(Flags::All);
	if ((registers_.af.GetHigh() + value) > 0xFF) SetFlag(Flags::C);
	if (((registers_.af.GetHigh() & 0x0F) + (value & 0x0F)) > 0x0F) SetFlag(Flags::H);
	registers_.af.GetHigh() += value;
	if (registers_.af.GetHigh() == 0) SetFlag(Flags::Z);
}

void CPU::Adc(uint8_t value)
{
	const auto carry_value = IsFlagSet(Flags::C) ? 1 : 0;
	ClearFlag(Flags::All);
	if ((registers_.af.GetHigh() + value + carry_value) > 0xFF) SetFlag(Flags::C);
	if (((registers_.af.GetHigh() & 0x0F) + (value & 0x0F) + carry_value) > 0x0F) SetFlag(Flags::H);
	registers_.af.GetHigh() += static_cast<uint8_t>(value + carry_value);
	if (registers_.af.GetHigh() == 0) SetFlag(Flags::Z);
}

void CPU::Sub(uint8_t value)
{
	ClearFlag(Flags::All);
	SetFlag(Flags::N);
	if ((registers_.af.GetHigh() - value) < 0) SetFlag(Flags::C);
	if (((registers_.af.GetHigh() & 0x0F) - (value & 0x0F)) < 0) SetFlag(Flags::H);
	registers_.af.GetHigh() -= value;
	if (registers_.af.GetHigh() == 0) SetFlag(Flags::Z);
}

void CPU::Sbc(uint8_t value)
{
	const auto carry_value = IsFlagSet(Flags::C) ? 1 : 0;
	ClearFlag(Flags::All);
	SetFlag(Flags::N);
	if ((registers_.af.GetHigh() - value - carry_value) < 0) SetFlag(Flags::C);
	if (((registers_.af.GetHigh() & 0x0F) - (value & 0x0F) - carry_value) < 0) SetFlag(Flags::H);
	registers_.af.GetHigh() -= static_cast<uint8_t>(value + carry_value);
	if (registers_.af.GetHigh() == 0) SetFlag(Flags::Z);
}

void CPU::And(uint8_t value)
{
	const uint8_t result = registers_.af.GetHigh() & value;
	ClearFlag(Flags::All);
	SetFlag(Flags::H);
	if (result == 0) SetFlag(Flags::Z);
	registers_.af.GetHigh() = result;
}

void CPU::Xor(uint8_t value)
{
	const uint8_t result = registers_.af.GetHigh() ^ value;
	ClearFlag(Flags::All);
	if (result == 0) SetFlag(Flags::Z);
	registers_.af.GetHigh() = result;
}

void CPU::Or(uint8_t value)
{
	const uint8_t result = registers_.af.GetHigh() | value;
	ClearFlag(Flags::All);
	if (result == 0) SetFlag(Flags::Z);
	registers_.af.GetHigh() = result;
}

void CPU::Compare(uint8_t value)
{
	ClearFlag(Flags::All);
	SetFlag(Flags::N);
	if ((registers_.af.GetHigh() - value) < 0) SetFlag(Flags::C);
	if (((registers_.af.GetHigh() & 0x0F) - (value & 0x0F)) < 0) SetFlag(Flags::H);
	if (registers_.af.GetHigh() == value) SetFlag(Flags::Z);
}

void CPU::AddToHl(uint16_t value)
{
	ClearFlag(Flags::All & ~Flags::Z);
	if ((registers_.hl + value) > 0xFFFF) SetFlag(Flags::C);
	if (((registers_.hl & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF) SetFlag(Flags::H);
	registers_.hl += value;
}

void CPU::Call(Memory::Address address)
{
	PushWordToStack(registers_.pc);
	registers_.pc = address;
}

void CPU::Return()
{
	registers_.pc = PopWordFromStack();
	NotifyMachineCycleLapse();
}
#pragma endregion

#pragma region CB instruction helper functions
void CPU::Rlc(uint8_t &reg)
{
	ClearFlag(Flags::All);
	if ((reg & 0x80) != 0) SetFlag(Flags::C);
	reg = (reg << 1) | ((reg & 0x80) >> 7);
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Rrc(uint8_t &reg)
{
	ClearFlag(Flags::All);
	if ((reg & 0x01) != 0) SetFlag(Flags::C);
	reg = (reg >> 1) | ((reg & 0x01) << 7);
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Rl(uint8_t &reg)
{
	const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
	ClearFlag(Flags::All);
	if ((reg & 0x80) != 0) SetFlag(Flags::C);
	reg = (reg << 1) | carry_value;
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Rr(uint8_t &reg)
{
	const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
	ClearFlag(Flags::All);
	if ((reg & 0x01) != 0) SetFlag(Flags::C);
	reg = (reg >> 1) | (carry_value << 7);
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Sla(uint8_t &reg)
{
	ClearFlag(Flags::All);
	if ((reg & 0x80) != 0) SetFlag(Flags::C);
	reg <<= 1;
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Sra(uint8_t &reg)
{
	ClearFlag(Flags::All);
	const auto current_bit7 = static_cast<uint8_t>(reg & 0x80);
	if ((reg & 0x01) != 0) SetFlag(Flags::C);
	reg = (reg >> 1) | current_bit7;
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::Swap(uint8_t &reg)
{
	ClearFlag(Flags::All);
	if (reg == 0) SetFlag(Flags::Z);
	reg = ((reg & 0xF0) >> 4) | ((reg & 0x0F) << 4);
}

void CPU::Srl(uint8_t &reg)
{
	ClearFlag(Flags::All);
	if ((reg & 0x01) != 0) SetFlag(Flags::C);
	reg >>= 1;
	if (reg == 0) SetFlag(Flags::Z);
}

void CPU::TestBit(uint8_t reg, int bit_num)
{
	ClearFlag(Flags::N | Flags::Z);
	SetFlag(Flags::H);
	if ((reg & (1 << bit_num)) == 0) SetFlag(Flags::Z);
}
#pragma endregion

#pragma region MMU mapped memory read/write functions
uint8_t CPU::OnIoMemoryRead(Memory::Address address) const
{
	assert(address == Memory::IF);
	return (0xE0 | requested_interrupts_);
}

void CPU::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	assert(address == Memory::IF);
	requested_interrupts_ = value & 0x1F;
}

uint8_t CPU::OnInterruptsRead(Memory::Address) const
{
	return (0xE0 | enabled_interrupts_);
}

void CPU::OnInterruptsWritten(Memory::Address, uint8_t value)
{
	enabled_interrupts_ = value & 0x1F;
}
#pragma endregion

#pragma region Listener notification
void CPU::NotifyRunningLoopInterruption() const
{
	for (auto& listener : listeners_)
	{
		listener->OnRunningLoopInterrupted();
	}
}

void CPU::NotifyMachineCycleLapse() const
{
	for (auto& listener : listeners_)
	{
		listener->OnMachineCycleLapse();
	}
}
#pragma endregion

#pragma region Flag operations
void CPU::SetFlag(Flags flag)
{
	registers_.af.GetLow() = static_cast<uint8_t>(ReadFlags() | flag);
}

void CPU::ClearFlag(Flags flag)
{
	registers_.af.GetLow() = static_cast<uint8_t>(ReadFlags() & ~flag);
}

void CPU::ToggleFlag(Flags flag)
{
	registers_.af.GetLow() = static_cast<uint8_t>(ReadFlags() ^ flag);
}

bool CPU::IsFlagSet(Flags flag) const
{
	return (static_cast<uint8_t>(ReadFlags() & flag)) != 0;
}

CPU::Flags CPU::ReadFlags() const
{
	return static_cast<Flags>(registers_.af.GetLow());
}
#pragma endregion
