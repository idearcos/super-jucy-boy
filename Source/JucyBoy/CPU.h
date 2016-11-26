#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <future>
#include <set>
#include "Registers.h"
#include "Memory.h"

class MMU;

class CPU
{
public:
	using MachineCycles = size_t;
	using OpCode = uint8_t;
	using Instruction = std::function<MachineCycles()>;
	using BreakpointList = std::set<uint16_t>;

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

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnExceptionInRunningLoop() {}
		virtual void OnCyclesLapsed(MachineCycles /*cycles*/) {}
		virtual void OnBreakpointsChanged(const BreakpointList &/*breakpoint_list*/) {}
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

	// Status retrieval
	Registers GetRegistersState() const { return registers_; }
	inline Flags GetFlagsState() const { return ReadFlags(); }

	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	// Initialization of instructions_ array
	void PopulateInstructions();
	void PopulateInstructionNames();
	void PopulateCbInstructions();
	void PopulateCbInstructionNames();

	// Execution flow
	inline OpCode FetchOpcode() { return FetchByte(); }
	MachineCycles ExecuteInstruction(OpCode opcode);
	void RunningLoopFunction();
	bool IsBreakpointHit() const;
	uint8_t FetchByte();
	uint16_t FetchWord();

	// Stack R/W
	//uint8_t ReadByteFromStack();
	uint16_t ReadWordFromStack();
	//void WriteByteToStack(uint8_t value);
	void WriteWordToStack(uint16_t value);

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
	void Return();

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
		if ((reg & (1 << BitNum)) != 0) SetFlag(Flags::Z);
	}

	// Flag operations
	void SetFlag(Flags flag);
	void ClearFlag(Flags flag);
	bool IsFlagSet(Flags flag) const;
	Flags ReadFlags() const;

	// Listener notification
	void NotifyBreakpointsChange() const;
	void NotifyExceptionInRunningLoop() const;
	void NotifyCyclesLapsed(MachineCycles cycles) const;

private:
	Registers registers_;
	uint16_t previous_pc_{ 0 };

	std::array<Instruction, 256> instructions_;
	std::array<std::string, 256> instruction_names_;
	std::array<Instruction, 256> cb_instructions_;
	std::array<std::string, 256> cb_instruction_names_;

	std::atomic<bool> exit_loop_{ false };
	std::future<void> loop_function_result_;

	bool interrupt_master_enable_{ true };
	bool ime_requested_{ false }; // Used to delay IME one instruction, since EI enables the interrupts for the instruction AFTER itself
	std::array<bool, 5> enabled_interrupts_{ false,false,false,false,false };
	std::array<bool, 5> requested_interrupts_{ true,false,false,false,false };

	BreakpointList breakpoints_;

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