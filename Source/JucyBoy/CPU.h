#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <thread>
#include "RegisterPair.h"

class MMU;

class CPU
{
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

	OpCode FetchOpcode();
	MachineCycles ExecuteInstruction(OpCode opcode);

private:
	void SetFlags(Flags flags) { af_.WriteLowByte(af_.ReadLowByte() | flags); }
	void ClearFlags(Flags flags) { af_.WriteLowByte(af_.ReadLowByte() & ~flags); }
	void ClearAndSetFlags(Flags flags) { af_.WriteLowByte(flags); }
	bool IsFlagSet(Flags flags) const { return (af_.ReadLowByte() & flags) != 0; }

private:
	std::array<Instruction, 256> instructions_;

	RegisterPair af_;
	RegisterPair bc_;
	RegisterPair de_;
	RegisterPair hl_;
	uint16_t pc_;
	uint16_t sp_;
	
	MMU *mmu_;
};
