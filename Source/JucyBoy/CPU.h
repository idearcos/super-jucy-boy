#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <future>
#include <set>
#include "RegisterPair.h"

class MMU;

class CPU
{
public:
	using MachineCycles = size_t;
	using OpCode = uint8_t;
	using Instruction = std::function<MachineCycles()>;

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
		virtual void OnExceptionInRunningLoop() = 0;
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
	OpCode FetchOpcode();
	MachineCycles ExecuteInstruction(OpCode opcode);
	void RunningLoopFunction();

	// Flag operations
	void SetFlag(Flags flag);
	void ClearFlag(Flags flag);
	bool IsFlagSet(Flags flag) const;
	Flags ReadFlags() const;
	void ClearAndSetFlags(Flags flags);

	// Listener notification
	void NotifyCpuStateChange() const;
	void NotifyExceptionInRunningLoop() const;

private:
	Registers registers_;

	std::array<Instruction, 256> instructions_;
	std::array<std::string, 256> instruction_names_;
	std::array<Instruction, 256> cb_instructions_;
	std::array<std::string, 256> cb_instruction_names_;

	std::atomic<bool> exit_loop_{ false };
	std::future<void> loop_function_result_;

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