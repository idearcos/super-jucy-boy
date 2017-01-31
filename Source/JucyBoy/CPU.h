#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <future>
#include <set>
#include <bitset>
#include "Registers.h"
#include "Memory.h"

class MMU;

class CPU
{
public:
	using MachineCycles = size_t;
	using OpCode = uint8_t;
	using Instruction = std::function<void()>;
	using BreakpointList = std::set<uint16_t>;
	using InstructionBreakpointList = std::set<OpCode>;

	struct Registers
	{
		RegisterPair af;
		RegisterPair bc;
		RegisterPair de;
		RegisterPair hl;
		uint16_t pc;
		uint16_t sp;
	};

	enum class Flags : uint8_t
	{
		None = 0x00,
		C = 1 << 4,
		H = 1 << 5,
		N = 1 << 6,
		Z = 1 << 7,
		All = 0xF0
	};

	enum Interrupt
	{
		VBlank = 0,
		LcdStat,
		Timer,
		Serial,
		Joypad
	};

	enum class State
	{
		Running,
		Halted,
		HaltBug,
		Stopped
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnRunningLoopInterrupted() {}
		virtual void OnMachineCycleLapse() {}
		virtual void OnBreakpointsChanged(const BreakpointList &/*breakpoint_list*/) {}
		virtual void OnInstructionBreakpointsChanged(const InstructionBreakpointList &/*instruction_breakpoint_list*/) {}
	};

public:
	CPU(MMU &mmu);
	~CPU();

	// Set initial state of registers_
	void Reset();

	// Execution flow control
	void Run();
	void Stop();
	bool IsRunning() const noexcept;
	void StepOver();

	// MMU listener functions
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);
	void OnInterruptsRegisterWritten(Memory::Address address, uint8_t value);

	// Breakpoints
	void AddBreakpoint(Memory::Address address);
	void RemoveBreakpoint(Memory::Address address);
	void AddInstructionBreakpoint(OpCode opcode);
	void RemoveInstructionBreakpoint(OpCode opcode);

	// Status retrieval
	Registers GetRegistersState() const { return registers_; }
	inline Flags GetFlagsState() const { return ReadFlags(); }

	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	// Initialization of instructions_ array
	void PopulateInstructions();
	void PopulateCbInstructions();
	void PopulateCbInstructionNames();

	// Execution flow
	inline OpCode FetchOpcode() { return FetchByte(); }
	inline void ExecuteInstruction(OpCode opcode) { instructions_[opcode](); }
	void RunningLoopFunction();
	bool IsBreakpointHit() const;
	bool IsInstructionBreakpointHit() const;
	bool IsWatchpointHit(OpCode next_opcode) const;

	// Memory R/W
	uint8_t FetchByte();
	uint16_t FetchWord();
	uint16_t PopWordFromStack();
	void PushWordToStack(uint16_t value);
	uint8_t ReadByte(uint16_t address) const;
	void WriteByte(uint16_t address, uint8_t value) const;

	// Interrupts
	void CheckInterrupts();

	// Instruction helper functions
	void IncrementRegister(uint8_t &reg);
	void DecrementRegister(uint8_t &reg);
	void Add(uint8_t value);
	void Adc(uint8_t value);
	void Sub(uint8_t value);
	void Sbc(uint8_t value);
	void And(uint8_t value);
	void Xor(uint8_t value);
	void Or(uint8_t value);
	void Compare(uint8_t value);
	void AddToHl(uint16_t value);
	void Call(Memory::Address address);
	inline void Return() { registers_.pc = PopWordFromStack(); NotifyMachineCycleLapse(); }

	// CB instruction helper functions
	void Rlc(uint8_t &reg); // Rotate left
	void Rrc(uint8_t &reg); // Rotate right
	void Rl(uint8_t &reg); // Rotate left through carry
	void Rr(uint8_t &reg); // Rotate right through carry
	void Sla(uint8_t &reg); // Shift left arithmetic
	void Sra(uint8_t &reg); // Shift right arithmetic
	void Swap(uint8_t &reg); // Exchange low and high nibbles
	void Srl(uint8_t &reg); // Shift right logical
	template <int BitNum>
	void TestBit(uint8_t reg)
	{
		ClearFlag(Flags::N | Flags::Z);
		SetFlag(Flags::H);
		if ((reg & (1 << BitNum)) == 0) SetFlag(Flags::Z);
	}

	// Flag operations
	void SetFlag(Flags flag);
	void ClearFlag(Flags flag);
	void ToggleFlag(Flags flag);
	bool IsFlagSet(Flags flag) const;
	Flags ReadFlags() const;

	// Listener notification
	void NotifyRunningLoopInterruption() const;
	void NotifyMachineCycleLapse() const;
	void NotifyBreakpointsChange() const;
	void NotifyInstructionBreakpointsChange() const;

private:
	Registers registers_;
	uint16_t previous_pc_{ 0 };

	State current_state_{ State::Running };

	std::array<Instruction, 256> instructions_;
	std::array<Instruction, 256> cb_instructions_;
	std::array<std::string, 256> cb_instruction_names_;

	std::atomic<bool> exit_loop_{ false };
	std::future<void> loop_function_result_;

	bool interrupt_master_enable_{ true };
	bool ime_requested_{ false }; // Used to delay IME one instruction, since EI enables the interrupts for the instruction AFTER itself
	std::bitset<5> enabled_interrupts_{ 0 };
	std::bitset<5> requested_interrupts_{ 0 };

	BreakpointList breakpoints_;
	InstructionBreakpointList instruction_breakpoints_;

	MMU *mmu_{ nullptr };
	std::set<Listener*> listeners_;

private:
	CPU(const CPU&) = delete;
	CPU(CPU&&) = delete;
	CPU& operator=(const CPU&) = delete;
	CPU& operator=(CPU&&) = delete;
};

#pragma region Flags bitwise operators
inline CPU::Flags operator | (const CPU::Flags &lhs, const CPU::Flags &rhs)
{
	return static_cast<CPU::Flags>(static_cast<std::underlying_type_t<CPU::Flags>>(lhs) | static_cast<std::underlying_type_t<CPU::Flags>>(rhs));
}

inline CPU::Flags operator & (const CPU::Flags &lhs, const CPU::Flags &rhs)
{
	return static_cast<CPU::Flags>(static_cast<std::underlying_type_t<CPU::Flags>>(lhs) & static_cast<std::underlying_type_t<CPU::Flags>>(rhs));
}

inline CPU::Flags operator ^ (const CPU::Flags &lhs, const CPU::Flags &rhs)
{
	return static_cast<CPU::Flags>(static_cast<std::underlying_type_t<CPU::Flags>>(lhs) ^ static_cast<std::underlying_type_t<CPU::Flags>>(rhs));
}

inline CPU::Flags operator ~ (const CPU::Flags &flag)
{
	return static_cast<CPU::Flags>(~static_cast<std::underlying_type_t<CPU::Flags>>(flag));
}

inline CPU::Flags& operator |= (CPU::Flags &lhs, const CPU::Flags &rhs)
{
	lhs = static_cast<CPU::Flags>(static_cast<std::underlying_type_t<CPU::Flags>>(lhs) | static_cast<std::underlying_type_t<CPU::Flags>>(rhs));
	return lhs;
}
#pragma endregion
