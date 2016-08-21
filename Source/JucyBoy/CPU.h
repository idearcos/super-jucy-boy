#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <thread>
#include "RegisterPair.h"

class MMU;

class CPU
{
	struct Registers
	{
		RegisterPair af_;
		RegisterPair bc_;
		RegisterPair de_;
		RegisterPair hl_;
		uint16_t pc_;
		uint16_t sp_;
	};

	enum Flags : uint8_t
	{
		Zero = 0x80,
		Subtract = 0x40,
		HalfCarry = 0x20,
		Carry = 0x10,
	};

public:
	using MachineCycles = size_t;
	using OpCode = uint8_t;
	using Instruction = std::function<MachineCycles()>;

	CPU(MMU &mmu);
	~CPU();

	void Reset();
	void PopulateInstructions();

	OpCode FetchOpcode();
	MachineCycles ExecuteInstruction(OpCode opcode);

	const Registers& GetRegisters() const { return registers_; }

private:
	void SetFlags(Flags flags) { registers_.af_.WriteLowByte(registers_.af_.ReadLowByte() | flags); }
	void ClearFlags(Flags flags) { registers_.af_.WriteLowByte(registers_.af_.ReadLowByte() & ~flags); }
	void ClearAndSetFlags(Flags flags) { registers_.af_.WriteLowByte(flags); }
	bool IsFlagSet(Flags flags) const { return (registers_.af_.ReadLowByte() & flags) != 0; }

private:
	std::array<Instruction, 256> instructions_;
	Registers registers_;
	
	MMU *mmu_;
};
