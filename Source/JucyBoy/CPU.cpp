#include "CPU.h"
#include "MMU.h"

CPU::CPU(MMU &mmu) :
	mmu_{ &mmu }
{

}

CPU::~CPU()
{
	
}

CPU::OpCode CPU::FetchOpcode()
{
	return mmu_->Read(pc_++);
}

CPU::MachineCycles CPU::ExecuteInstruction(OpCode opcode)
{
	return instructions_[opcode]();
}
