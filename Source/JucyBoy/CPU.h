#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <future>
#include <set>
#include "Registers.h"

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
	};

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnCpuStateChanged(const Registers &registers, Flags flags) = 0;
		virtual void OnBreakpointsChanged(const BreakpointList &breakpoint_list) = 0;
		virtual void OnRunningLoopExited() = 0;
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

	void AddBreakpoint(uint16_t address);
	void RemoveBreakpoint(uint16_t address);

	// Listeners management
	void AddListener(Listener &listener);
	void RemoveListener(Listener &listener);

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
	uint8_t FetchByte();
	uint16_t FetchWord();
	bool BreakpointHit() const;

	// Flag operations
	void SetFlag(Flags flag);
	void ClearFlag(Flags flag);
	bool IsFlagSet(Flags flag) const;
	Flags ReadFlags() const;
	void ClearAndSetFlags(Flags flags);

	// Listener notification
	void NotifyCpuStateChange() const;
	void NotifyBreakpointsChange() const;
	void NotifyRunningLoopExited() const;

private:
	Registers registers_;
	uint16_t previous_pc_{ 0 };

	std::array<Instruction, 256> instructions_;
	std::array<std::string, 256> instruction_names_;
	std::array<Instruction, 256> cb_instructions_;
	std::array<std::string, 256> cb_instruction_names_;

	std::atomic<bool> exit_loop_{ false };
	std::future<void> loop_function_result_;

	BreakpointList breakpoints_;

	MMU *mmu_;
	std::set<Listener*> listeners_;
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