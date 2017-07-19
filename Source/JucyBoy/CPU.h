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
	using OpCode = uint8_t;

	struct Registers
	{
		RegisterPair af{ 0x01B0 };
		RegisterPair bc{ 0x0013 };
		RegisterPair de{ 0x00D8 };
		RegisterPair hl{ 0x014D };
		uint16_t pc{ 0x0100 };
		uint16_t sp{ 0xFFFE };
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

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnRunningLoopInterrupted() {}
		virtual void OnMachineCycleLapse() {}
	};

public:
	CPU(MMU &mmu);
	virtual ~CPU();

	// Set initial state of registers_
	void Reset();

	// Execution flow control
	void Run();
	void Stop();
	bool IsRunning() const noexcept;
	void StepOver();

	// MMU mapped memory read/write functions
	uint8_t OnIoMemoryRead(Memory::Address address) const;
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);
	uint8_t OnInterruptsRead(Memory::Address address) const;
	void OnInterruptsWritten(Memory::Address address, uint8_t value);

	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

protected:
	void ExecuteOneInstruction();

	// Listener notification
	void NotifyRunningLoopInterruption() const;

	Flags ReadFlags() const;

private:
	// Initialization of instructions_ array
	void PopulateInstructions();
	void PopulateCbInstructions();

	// Execution flow
	inline OpCode FetchOpcode() { return FetchByte(); }
	inline void ExecuteInstruction(OpCode opcode) { instructions_[opcode](); }
	void RunningLoopFunction();

	// Interrupts
	void CheckInterrupts();

	// Memory R/W
	inline uint8_t FetchByte() { return ReadByte(registers_.pc++); }
	uint16_t FetchWord();
	uint16_t PopWordFromStack();
	void PushWordToStack(uint16_t value);
	uint8_t ReadByte(uint16_t address) const;
	void WriteByte(uint16_t address, uint8_t value) const;

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
	void TestBit(uint8_t reg, int bit_num);

	// Flag operations
	void SetFlag(Flags flag);
	void ClearFlag(Flags flag);
	void ToggleFlag(Flags flag);
	bool IsFlagSet(Flags flag) const;

	// Listener notification
	void NotifyMachineCycleLapse() const;

protected:
	Registers registers_;

	std::atomic<bool> exit_loop_{ false };
	std::future<void> loop_function_result_;

	MMU *mmu_{ nullptr };

private:
	using Instruction = std::function<void()>;

	enum class Interrupt
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

	uint16_t previous_pc_{ 0 };

	State current_state_{ State::Running };

	std::array<Instruction, 256> instructions_;
	std::array<Instruction, 256> cb_instructions_;

	bool interrupt_master_enable_{ true };
	bool ime_requested_{ false }; // Used to delay IME one instruction, since EI enables the interrupts for the instruction AFTER itself
	uint8_t enabled_interrupts_{ 0 };
	uint8_t requested_interrupts_{ 0 };

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
