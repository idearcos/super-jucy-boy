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
	
}

void CPU::Reset()
{
	registers_.af_.WriteWord(0x01B0);
	registers_.bc_.WriteWord(0x0013);
	registers_.de_.WriteWord(0x00D8);
	registers_.hl_.WriteWord(0x014D);
	registers_.pc_ = 0x0100;
	registers_.sp_ = 0xFFFE;
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

CPU::OpCode CPU::FetchOpcode()
{
	return mmu_->Read(registers_.pc_++);
}

CPU::MachineCycles CPU::ExecuteInstruction(OpCode opcode)
{
	return instructions_[opcode]();
}
