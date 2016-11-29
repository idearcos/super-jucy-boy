#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"

void CPU::PopulateCbInstructions()
{
	for (size_t ii = 0; ii < instructions_.size(); ++ii)
	{
		cb_instructions_[ii] = [this, ii]() -> MachineCycles {
			registers_.pc = previous_pc_;
			std::stringstream error;
			error << "CB Instruction not yet implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << cb_instruction_names_[ii] << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc << ")";
			throw std::runtime_error(error.str());
		};
	}

#pragma region CB instructions 0x00 - 0x0F
	// RLC B
	cb_instructions_[0x00] = [this]() -> MachineCycles {
		Rlc(registers_.bc.GetHigh());
		return 2;
	};

	// RLC C
	cb_instructions_[0x01] = [this]() -> MachineCycles {
		Rlc(registers_.bc.GetLow());
		return 2;
	};

	// RLC D
	cb_instructions_[0x02] = [this]() -> MachineCycles {
		Rlc(registers_.de.GetHigh());
		return 2;
	};

	// RLC E
	cb_instructions_[0x03] = [this]() -> MachineCycles {
		Rlc(registers_.de.GetLow());
		return 2;
	};

	// RLC H
	cb_instructions_[0x04] = [this]() -> MachineCycles {
		Rlc(registers_.hl.GetHigh());
		return 2;
	};

	// RLC L
	cb_instructions_[0x05] = [this]() -> MachineCycles {
		Rlc(registers_.hl.GetLow());
		return 2;
	};

	// RLC (HL)
	cb_instructions_[0x06] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Rlc(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// RLC A
	cb_instructions_[0x07] = [this]() -> MachineCycles {
		Rlc(registers_.af.GetHigh());
		return 2;
	};

	// RRC B
	cb_instructions_[0x08] = [this]() -> MachineCycles {
		Rrc(registers_.bc.GetHigh());
		return 2;
	};

	// RRC C
	cb_instructions_[0x09] = [this]() -> MachineCycles {
		Rrc(registers_.bc.GetLow());
		return 2;
	};

	// RRC D
	cb_instructions_[0x0A] = [this]() -> MachineCycles {
		Rrc(registers_.de.GetHigh());
		return 2;
	};

	// RRC E
	cb_instructions_[0x0B] = [this]() -> MachineCycles {
		Rrc(registers_.de.GetLow());
		return 2;
	};

	// RRC H
	cb_instructions_[0x0C] = [this]() -> MachineCycles {
		Rrc(registers_.hl.GetHigh());
		return 2;
	};

	// RRC L
	cb_instructions_[0x0D] = [this]() -> MachineCycles {
		Rrc(registers_.hl.GetLow());
		return 2;
	};

	// RRC (HL)
	cb_instructions_[0x0E] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Rrc(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// RRC A
	cb_instructions_[0x0F] = [this]() -> MachineCycles {
		Rrc(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x10 - 0x1F
	// RL B
	cb_instructions_[0x10] = [this]() -> MachineCycles {
		Rl(registers_.bc.GetHigh());
		return 2;
	};

	// RL C
	cb_instructions_[0x11] = [this]() -> MachineCycles {
		Rl(registers_.bc.GetLow());
		return 2;
	};

	// RL D
	cb_instructions_[0x12] = [this]() -> MachineCycles {
		Rl(registers_.de.GetHigh());
		return 2;
	};

	// RL E
	cb_instructions_[0x13] = [this]() -> MachineCycles {
		Rl(registers_.de.GetLow());
		return 2;
	};

	// RL H
	cb_instructions_[0x14] = [this]() -> MachineCycles {
		Rl(registers_.hl.GetHigh());
		return 2;
	};

	// RL L
	cb_instructions_[0x15] = [this]() -> MachineCycles {
		Rl(registers_.hl.GetLow());
		return 2;
	};

	// RL (HL)
	cb_instructions_[0x16] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Rl(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// RL A
	cb_instructions_[0x17] = [this]() -> MachineCycles {
		Rl(registers_.af.GetHigh());
		return 2;
	};

	// RR B
	cb_instructions_[0x18] = [this]() -> MachineCycles {
		Rr(registers_.bc.GetHigh());
		return 2;
	};

	// RR C
	cb_instructions_[0x19] = [this]() -> MachineCycles {
		Rr(registers_.bc.GetLow());
		return 2;
	};

	// RR D
	cb_instructions_[0x1A] = [this]() -> MachineCycles {
		Rr(registers_.de.GetHigh());
		return 2;
	};

	// RR E
	cb_instructions_[0x1B] = [this]() -> MachineCycles {
		Rr(registers_.de.GetLow());
		return 2;
	};

	// RR H
	cb_instructions_[0x1C] = [this]() -> MachineCycles {
		Rr(registers_.hl.GetHigh());
		return 2;
	};

	// RR L
	cb_instructions_[0x1D] = [this]() -> MachineCycles {
		Rr(registers_.hl.GetLow());
		return 2;
	};

	// RR (HL)
	cb_instructions_[0x1E] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Rr(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// RR A
	cb_instructions_[0x1F] = [this]() -> MachineCycles {
		Rr(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x20 - 0x2F
	// SLA B
	cb_instructions_[0x20] = [this]() -> MachineCycles {
		Sla(registers_.bc.GetHigh());
		return 2;
	};

	// SLA C
	cb_instructions_[0x21] = [this]() -> MachineCycles {
		Sla(registers_.bc.GetLow());
		return 2;
	};

	// SLA D
	cb_instructions_[0x22] = [this]() -> MachineCycles {
		Sla(registers_.de.GetHigh());
		return 2;
	};

	// SLA E
	cb_instructions_[0x23] = [this]() -> MachineCycles {
		Sla(registers_.de.GetLow());
		return 2;
	};

	// SLA H
	cb_instructions_[0x24] = [this]() -> MachineCycles {
		Sla(registers_.hl.GetHigh());
		return 2;
	};

	// SLA L
	cb_instructions_[0x25] = [this]() -> MachineCycles {
		Sla(registers_.hl.GetLow());
		return 2;
	};

	// SLA (HL)
	cb_instructions_[0x26] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Sla(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// SLA A
	cb_instructions_[0x27] = [this]() -> MachineCycles {
		Sla(registers_.af.GetHigh());
		return 2;
	};

	// SRA B
	cb_instructions_[0x28] = [this]() -> MachineCycles {
		Sra(registers_.bc.GetHigh());
		return 2;
	};

	// SRA C
	cb_instructions_[0x29] = [this]() -> MachineCycles {
		Sra(registers_.bc.GetLow());
		return 2;
	};

	// SRA D
	cb_instructions_[0x2A] = [this]() -> MachineCycles {
		Sra(registers_.de.GetHigh());
		return 2;
	};

	// SRA E
	cb_instructions_[0x2B] = [this]() -> MachineCycles {
		Sra(registers_.de.GetLow());
		return 2;
	};

	// SRA H
	cb_instructions_[0x2C] = [this]() -> MachineCycles {
		Sra(registers_.hl.GetHigh());
		return 2;
	};

	// SRA L
	cb_instructions_[0x2D] = [this]() -> MachineCycles {
		Sra(registers_.hl.GetLow());
		return 2;
	};

	// SRA (HL)
	cb_instructions_[0x2E] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Sra(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// SRA A
	cb_instructions_[0x2F] = [this]() -> MachineCycles {
		Sra(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x30 - 0x3F
	// SWAP B
	cb_instructions_[0x30] = [this]() -> MachineCycles {
		Swap(registers_.bc.GetHigh());
		return 2;
	};

	// SWAP C
	cb_instructions_[0x31] = [this]() -> MachineCycles {
		Swap(registers_.bc.GetLow());
		return 2;
	};

	// SWAP D
	cb_instructions_[0x32] = [this]() -> MachineCycles {
		Swap(registers_.de.GetHigh());
		return 2;
	};

	// SWAP E
	cb_instructions_[0x33] = [this]() -> MachineCycles {
		Swap(registers_.de.GetLow());
		return 2;
	};

	// SWAP H
	cb_instructions_[0x34] = [this]() -> MachineCycles {
		Swap(registers_.hl.GetHigh());
		return 2;
	};

	// SWAP L
	cb_instructions_[0x35] = [this]() -> MachineCycles {
		Swap(registers_.hl.GetLow());
		return 2;
	};

	// SWAP (HL)
	cb_instructions_[0x36] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Swap(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// SWAP A
	cb_instructions_[0x37] = [this]() -> MachineCycles {
		Swap(registers_.af.GetHigh());
		return 2;
	};

	// SRL B
	cb_instructions_[0x38] = [this]() -> MachineCycles {
		Srl(registers_.bc.GetHigh());
		return 2;
	};

	// SRL C
	cb_instructions_[0x39] = [this]() -> MachineCycles {
		Srl(registers_.bc.GetLow());
		return 2;
	};

	// SRL D
	cb_instructions_[0x3A] = [this]() -> MachineCycles {
		Srl(registers_.de.GetHigh());
		return 2;
	};

	// SRL E
	cb_instructions_[0x3B] = [this]() -> MachineCycles {
		Srl(registers_.de.GetLow());
		return 2;
	};

	// SRL H
	cb_instructions_[0x3C] = [this]() -> MachineCycles {
		Srl(registers_.hl.GetHigh());
		return 2;
	};

	// SRL L
	cb_instructions_[0x3D] = [this]() -> MachineCycles {
		Srl(registers_.hl.GetLow());
		return 2;
	};

	// SRL (HL)
	cb_instructions_[0x3E] = [this]() -> MachineCycles {
		auto value = mmu_->ReadByte(registers_.hl);
		Srl(value);
		mmu_->WriteByte(registers_.hl, value);
		return 4;
	};

	// SRL A
	cb_instructions_[0x3F] = [this]() -> MachineCycles {
		Srl(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x40 - 0x4F
	// BIT 0, B
	cb_instructions_[0x40] = [this]() -> MachineCycles {
		TestBit<0>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 0, C
	cb_instructions_[0x41] = [this]() -> MachineCycles {
		TestBit<0>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 0, D
	cb_instructions_[0x42] = [this]() -> MachineCycles {
		TestBit<0>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 0, E
	cb_instructions_[0x43] = [this]() -> MachineCycles {
		TestBit<0>(registers_.de.GetLow());
		return 2;
	};

	// BIT 0, H
	cb_instructions_[0x44] = [this]() -> MachineCycles {
		TestBit<0>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 0, L
	cb_instructions_[0x45] = [this]() -> MachineCycles {
		TestBit<0>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 0, (HL)
	cb_instructions_[0x46] = [this]() -> MachineCycles {
		TestBit<0>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 0, A
	cb_instructions_[0x47] = [this]() -> MachineCycles {
		TestBit<0>(registers_.af.GetHigh());
		return 2;
	};

	// BIT 1, B
	cb_instructions_[0x48] = [this]() -> MachineCycles {
		TestBit<1>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 1, C
	cb_instructions_[0x49] = [this]() -> MachineCycles {
		TestBit<1>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 1, D
	cb_instructions_[0x4A] = [this]() -> MachineCycles {
		TestBit<1>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 1, E
	cb_instructions_[0x4B] = [this]() -> MachineCycles {
		TestBit<1>(registers_.de.GetLow());
		return 2;
	};

	// BIT 1, H
	cb_instructions_[0x4C] = [this]() -> MachineCycles {
		TestBit<1>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 1, L
	cb_instructions_[0x4D] = [this]() -> MachineCycles {
		TestBit<1>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 1, (HL)
	cb_instructions_[0x4E] = [this]() -> MachineCycles {
		TestBit<1>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 1, A
	cb_instructions_[0x4F] = [this]() -> MachineCycles {
		TestBit<1>(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x50 - 0x5F
	// BIT 2, B
	cb_instructions_[0x50] = [this]() -> MachineCycles {
		TestBit<2>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 2, C
	cb_instructions_[0x51] = [this]() -> MachineCycles {
		TestBit<2>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 2, D
	cb_instructions_[0x52] = [this]() -> MachineCycles {
		TestBit<2>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 2, E
	cb_instructions_[0x53] = [this]() -> MachineCycles {
		TestBit<2>(registers_.de.GetLow());
		return 2;
	};

	// BIT 2, H
	cb_instructions_[0x54] = [this]() -> MachineCycles {
		TestBit<2>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 2, L
	cb_instructions_[0x55] = [this]() -> MachineCycles {
		TestBit<2>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 2, (HL)
	cb_instructions_[0x56] = [this]() -> MachineCycles {
		TestBit<2>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 2, A
	cb_instructions_[0x57] = [this]() -> MachineCycles {
		TestBit<2>(registers_.af.GetHigh());
		return 2;
	};

	// BIT 3, B
	cb_instructions_[0x58] = [this]() -> MachineCycles {
		TestBit<3>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 3, C
	cb_instructions_[0x59] = [this]() -> MachineCycles {
		TestBit<3>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 3, D
	cb_instructions_[0x5A] = [this]() -> MachineCycles {
		TestBit<3>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 3, E
	cb_instructions_[0x5B] = [this]() -> MachineCycles {
		TestBit<3>(registers_.de.GetLow());
		return 2;
	};

	// BIT 3, H
	cb_instructions_[0x5C] = [this]() -> MachineCycles {
		TestBit<3>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 3, L
	cb_instructions_[0x5D] = [this]() -> MachineCycles {
		TestBit<3>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 3, (HL)
	cb_instructions_[0x5E] = [this]() -> MachineCycles {
		TestBit<3>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 3, A
	cb_instructions_[0x5F] = [this]() -> MachineCycles {
		TestBit<3>(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x60 - 0x6F
	// BIT 4, B
	cb_instructions_[0x60] = [this]() -> MachineCycles {
		TestBit<4>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 4, C
	cb_instructions_[0x61] = [this]() -> MachineCycles {
		TestBit<4>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 4, D
	cb_instructions_[0x62] = [this]() -> MachineCycles {
		TestBit<4>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 4, E
	cb_instructions_[0x63] = [this]() -> MachineCycles {
		TestBit<4>(registers_.de.GetLow());
		return 2;
	};

	// BIT 4, H
	cb_instructions_[0x64] = [this]() -> MachineCycles {
		TestBit<4>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 4, L
	cb_instructions_[0x65] = [this]() -> MachineCycles {
		TestBit<4>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 4, (HL)
	cb_instructions_[0x66] = [this]() -> MachineCycles {
		TestBit<4>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 4, A
	cb_instructions_[0x67] = [this]() -> MachineCycles {
		TestBit<4>(registers_.af.GetHigh());
		return 2;
	};

	// BIT 5, B
	cb_instructions_[0x68] = [this]() -> MachineCycles {
		TestBit<5>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 5, C
	cb_instructions_[0x69] = [this]() -> MachineCycles {
		TestBit<5>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 5, D
	cb_instructions_[0x6A] = [this]() -> MachineCycles {
		TestBit<5>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 5, E
	cb_instructions_[0x6B] = [this]() -> MachineCycles {
		TestBit<5>(registers_.de.GetLow());
		return 2;
	};

	// BIT 5, H
	cb_instructions_[0x6C] = [this]() -> MachineCycles {
		TestBit<5>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 5, L
	cb_instructions_[0x6D] = [this]() -> MachineCycles {
		TestBit<5>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 5, (HL)
	cb_instructions_[0x6E] = [this]() -> MachineCycles {
		TestBit<5>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 5, A
	cb_instructions_[0x6F] = [this]() -> MachineCycles {
		TestBit<5>(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x70 - 0x7F
	// BIT 6, B
	cb_instructions_[0x70] = [this]() -> MachineCycles {
		TestBit<6>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 6, C
	cb_instructions_[0x71] = [this]() -> MachineCycles {
		TestBit<6>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 6, D
	cb_instructions_[0x72] = [this]() -> MachineCycles {
		TestBit<6>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 6, E
	cb_instructions_[0x73] = [this]() -> MachineCycles {
		TestBit<6>(registers_.de.GetLow());
		return 2;
	};

	// BIT 6, H
	cb_instructions_[0x74] = [this]() -> MachineCycles {
		TestBit<6>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 6, L
	cb_instructions_[0x75] = [this]() -> MachineCycles {
		TestBit<6>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 6, (HL)
	cb_instructions_[0x76] = [this]() -> MachineCycles {
		TestBit<6>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 6, A
	cb_instructions_[0x77] = [this]() -> MachineCycles {
		TestBit<6>(registers_.af.GetHigh());
		return 2;
	};

	// BIT 7, B
	cb_instructions_[0x78] = [this]() -> MachineCycles {
		TestBit<7>(registers_.bc.GetHigh());
		return 2;
	};

	// BIT 7, C
	cb_instructions_[0x79] = [this]() -> MachineCycles {
		TestBit<7>(registers_.bc.GetLow());
		return 2;
	};

	// BIT 7, D
	cb_instructions_[0x7A] = [this]() -> MachineCycles {
		TestBit<7>(registers_.de.GetHigh());
		return 2;
	};

	// BIT 7, E
	cb_instructions_[0x7B] = [this]() -> MachineCycles {
		TestBit<7>(registers_.de.GetLow());
		return 2;
	};

	// BIT 7, H
	cb_instructions_[0x7C] = [this]() -> MachineCycles {
		TestBit<7>(registers_.hl.GetHigh());
		return 2;
	};

	// BIT 7, L
	cb_instructions_[0x7D] = [this]() -> MachineCycles {
		TestBit<7>(registers_.hl.GetLow());
		return 2;
	};

	// BIT 7, (HL)
	cb_instructions_[0x7E] = [this]() -> MachineCycles {
		TestBit<7>(mmu_->ReadByte(registers_.hl));
		return 4;
	};

	// BIT 7, A
	cb_instructions_[0x7F] = [this]() -> MachineCycles {
		TestBit<7>(registers_.af.GetHigh());
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x80 - 0x8F
	// RES 0, B
	cb_instructions_[0x80] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 0);
		return 2;
	};

	// RES 0, C
	cb_instructions_[0x81] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 0);
		return 2;
	};

	// RES 0, D
	cb_instructions_[0x82] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 0);
		return 2;
	};

	// RES 0, E
	cb_instructions_[0x83] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 0);
		return 2;
	};

	// RES 0, H
	cb_instructions_[0x84] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 0);
		return 2;
	};

	// RES 0, L
	cb_instructions_[0x85] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 0);
		return 2;
	};

	// RES 0, (HL)
	cb_instructions_[0x86] = [this]() -> MachineCycles {
		mmu_->ClearBit<0>(registers_.hl);
		return 4;
	};

	// RES 0, A
	cb_instructions_[0x87] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 0);
		return 2;
	};

	// RES 1, B
	cb_instructions_[0x88] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 1);
		return 2;
	};

	// RES 1, C
	cb_instructions_[0x89] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 1);
		return 2;
	};

	// RES 1, D
	cb_instructions_[0x8A] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 1);
		return 2;
	};

	// RES 1, E
	cb_instructions_[0x8B] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 1);
		return 2;
	};

	// RES 1, H
	cb_instructions_[0x8C] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 1);
		return 2;
	};

	// RES 1, L
	cb_instructions_[0x8D] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 1);
		return 2;
	};

	// RES 1, (HL)
	cb_instructions_[0x8E] = [this]() -> MachineCycles {
		mmu_->ClearBit<1>(registers_.hl);
		return 4;
	};

	// RES 1, A
	cb_instructions_[0x8F] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 1);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0x90 - 0x9F
	// RES 2, B
	cb_instructions_[0x90] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 2);
		return 2;
	};

	// RES 2, C
	cb_instructions_[0x91] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 2);
		return 2;
	};

	// RES 2, D
	cb_instructions_[0x92] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 2);
		return 2;
	};

	// RES 2, E
	cb_instructions_[0x93] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 2);
		return 2;
	};

	// RES 2, H
	cb_instructions_[0x94] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 2);
		return 2;
	};

	// RES 2, L
	cb_instructions_[0x95] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 2);
		return 2;
	};

	// RES 2, (HL)
	cb_instructions_[0x96] = [this]() -> MachineCycles {
		mmu_->ClearBit<2>(registers_.hl);
		return 4;
	};

	// RES 2, A
	cb_instructions_[0x97] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 2);
		return 2;
	};

	// RES 3, B
	cb_instructions_[0x98] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 3);
		return 2;
	};

	// RES 3, C
	cb_instructions_[0x99] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 3);
		return 2;
	};

	// RES 3, D
	cb_instructions_[0x9A] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 3);
		return 2;
	};

	// RES 3, E
	cb_instructions_[0x9B] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 3);
		return 2;
	};

	// RES 3, H
	cb_instructions_[0x9C] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 3);
		return 2;
	};

	// RES 3, L
	cb_instructions_[0x9D] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 3);
		return 2;
	};

	// RES 3, (HL)
	cb_instructions_[0x9E] = [this]() -> MachineCycles {
		mmu_->ClearBit<3>(registers_.hl);
		return 4;
	};

	// RES 3, A
	cb_instructions_[0x9F] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 3);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xA0 - 0xAF
	// RES 4, B
	cb_instructions_[0xA0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 4);
		return 2;
	};

	// RES 4, C
	cb_instructions_[0xA1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 4);
		return 2;
	};

	// RES 4, D
	cb_instructions_[0xA2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 4);
		return 2;
	};

	// RES 4, E
	cb_instructions_[0xA3] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 4);
		return 2;
	};

	// RES 4, H
	cb_instructions_[0xA4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 4);
		return 2;
	};

	// RES 4, L
	cb_instructions_[0xA5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 4);
		return 2;
	};

	// RES 4, (HL)
	cb_instructions_[0xA6] = [this]() -> MachineCycles {
		mmu_->ClearBit<4>(registers_.hl);
		return 4;
	};

	// RES 4, A
	cb_instructions_[0xA7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 4);
		return 2;
	};

	// RES 5, B
	cb_instructions_[0xA8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 5);
		return 2;
	};

	// RES 5, C
	cb_instructions_[0xA9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 5);
		return 2;
	};

	// RES 5, D
	cb_instructions_[0xAA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 5);
		return 2;
	};

	// RES 5, E
	cb_instructions_[0xAB] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 5);
		return 2;
	};

	// RES 5, H
	cb_instructions_[0xAC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 5);
		return 2;
	};

	// RES 5, L
	cb_instructions_[0xAD] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 5);
		return 2;
	};

	// RES 5, (HL)
	cb_instructions_[0xAE] = [this]() -> MachineCycles {
		mmu_->ClearBit<5>(registers_.hl);
		return 4;
	};

	// RES 5, A
	cb_instructions_[0xAF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 5);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xB0 - 0xBF
	// RES 6, B
	cb_instructions_[0xB0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 6);
		return 2;
	};

	// RES 6, C
	cb_instructions_[0xB1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 6);
		return 2;
	};

	// RES 6, D
	cb_instructions_[0xB2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 6);
		return 2;
	};

	// RES 6, E
	cb_instructions_[0xB3] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 6);
		return 2;
	};

	// RES 6, H
	cb_instructions_[0xB4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 6);
		return 2;
	};

	// RES 6, L
	cb_instructions_[0xB5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 6);
		return 2;
	};

	// RES 6, (HL)
	cb_instructions_[0xB6] = [this]() -> MachineCycles {
		mmu_->ClearBit<6>(registers_.hl);
		return 4;
	};

	// RES 6, A
	cb_instructions_[0xB7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 6);
		return 2;
	};

	// RES 7, B
	cb_instructions_[0xB8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() &= ~(1 << 7);
		return 2;
	};

	// RES 7, C
	cb_instructions_[0xB9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() &= ~(1 << 7);
		return 2;
	};

	// RES 7, D
	cb_instructions_[0xBA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() &= ~(1 << 7);
		return 2;
	};

	// RES 7, E
	cb_instructions_[0xBB] = [this]() -> MachineCycles {
		registers_.de.GetLow() &= ~(1 << 7);
		return 2;
	};

	// RES 7, H
	cb_instructions_[0xBC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() &= ~(1 << 7);
		return 2;
	};

	// RES 7, L
	cb_instructions_[0xBD] = [this]() -> MachineCycles {
		registers_.hl.GetLow() &= ~(1 << 7);
		return 2;
	};

	// RES 7, (HL)
	cb_instructions_[0xBE] = [this]() -> MachineCycles {
		mmu_->ClearBit<7>(registers_.hl);
		return 4;
	};

	// RES 7, A
	cb_instructions_[0xBF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() &= ~(1 << 7);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xC0 - 0xCF
	// SET 0, B
	cb_instructions_[0xC0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 0);
		return 2;
	};

	// SET 0, C
	cb_instructions_[0xC1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 0);
		return 2;
	};

	// SET 0, D
	cb_instructions_[0xC2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 0);
		return 2;
	};

	// SET 0, E
	cb_instructions_[0xC3] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 0);
		return 2;
	};

	// SET 0, H
	cb_instructions_[0xC4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 0);
		return 2;
	};

	// SET 0, L
	cb_instructions_[0xC5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 0);
		return 2;
	};

	// SET 0, (HL)
	cb_instructions_[0xC6] = [this]() -> MachineCycles {
		mmu_->SetBit<0>(registers_.hl);
		return 4;
	};

	// SET 0, A
	cb_instructions_[0xC7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 0);
		return 2;
	};

	// SET 1, B
	cb_instructions_[0xC8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 1);
		return 2;
	};

	// SET 1, C
	cb_instructions_[0xC9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 1);
		return 2;
	};

	// SET 1, D
	cb_instructions_[0xCA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 1);
		return 2;
	};

	// SET 1, E
	cb_instructions_[0xCB] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 1);
		return 2;
	};

	// SET 1, H
	cb_instructions_[0xCC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 1);
		return 2;
	};

	// SET 1, L
	cb_instructions_[0xCD] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 1);
		return 2;
	};

	// SET 1, (HL)
	cb_instructions_[0xCE] = [this]() -> MachineCycles {
		mmu_->SetBit<1>(registers_.hl);
		return 4;
	};

	// SET 1, A
	cb_instructions_[0xCF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 1);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xD0 - 0xDF
	// SET 2, B
	cb_instructions_[0xD0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 2);
		return 2;
	};

	// SET 2, C
	cb_instructions_[0xD1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 2);
		return 2;
	};

	// SET 2, D
	cb_instructions_[0xD2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 2);
		return 2;
	};

	// SET 2, E
	cb_instructions_[0xD3] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 2);
		return 2;
	};

	// SET 2, H
	cb_instructions_[0xD4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 2);
		return 2;
	};

	// SET 2, L
	cb_instructions_[0xD5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 2);
		return 2;
	};

	// SET 2, (HL)
	cb_instructions_[0xD6] = [this]() -> MachineCycles {
		mmu_->SetBit<2>(registers_.hl);
		return 4;
	};

	// SET 2, A
	cb_instructions_[0xD7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 2);
		return 2;
	};

	// SET 3, B
	cb_instructions_[0xD8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 3);
		return 2;
	};

	// SET 3, C
	cb_instructions_[0xD9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 3);
		return 2;
	};

	// SET 3, D
	cb_instructions_[0xDA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 3);
		return 2;
	};

	// SET 3, E
	cb_instructions_[0xDB] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 3);
		return 2;
	};

	// SET 3, H
	cb_instructions_[0xDC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 3);
		return 2;
	};

	// SET 3, L
	cb_instructions_[0xDD] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 3);
		return 2;
	};

	// SET 3, (HL)
	cb_instructions_[0xDE] = [this]() -> MachineCycles {
		mmu_->SetBit<3>(registers_.hl);
		return 4;
	};

	// SET 3, A
	cb_instructions_[0xDF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 3);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xE0 - 0xEF
	// SET 4, B
	cb_instructions_[0xE0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 4);
		return 2;
	};

	// SET 4, C
	cb_instructions_[0xE1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 4);
		return 2;
	};

	// SET 4, D
	cb_instructions_[0xE2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 4);
		return 2;
	};

	// SET 4, E
	cb_instructions_[0xE3] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 4);
		return 2;
	};

	// SET 4, H
	cb_instructions_[0xE4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 4);
		return 2;
	};

	// SET 4, L
	cb_instructions_[0xE5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 4);
		return 2;
	};

	// SET 4, (HL)
	cb_instructions_[0xE6] = [this]() -> MachineCycles {
		mmu_->SetBit<4>(registers_.hl);
		return 4;
	};

	// SET 4, A
	cb_instructions_[0xE7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 4);
		return 2;
	};

	// SET 5, B
	cb_instructions_[0xE8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 5);
		return 2;
	};

	// SET 5, C
	cb_instructions_[0xE9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 5);
		return 2;
	};

	// SET 5, D
	cb_instructions_[0xEA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 5);
		return 2;
	};

	// SET 5, E
	cb_instructions_[0xEB] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 5);
		return 2;
	};

	// SET 5, H
	cb_instructions_[0xEC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 5);
		return 2;
	};

	// SET 5, L
	cb_instructions_[0xED] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 5);
		return 2;
	};

	// SET 5, (HL)
	cb_instructions_[0xEE] = [this]() -> MachineCycles {
		mmu_->SetBit<5>(registers_.hl);
		return 4;
	};

	// SET 5, A
	cb_instructions_[0xEF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 5);
		return 2;
	};
#pragma endregion

#pragma region CB instructions 0xF0 - 0xFF
	// SET 6, B
	cb_instructions_[0xF0] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 6);
		return 2;
	};

	// SET 6, C
	cb_instructions_[0xF1] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 6);
		return 2;
	};

	// SET 6, D
	cb_instructions_[0xF2] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 6);
		return 2;
	};

	// SET 6, E
	cb_instructions_[0xF3] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 6);
		return 2;
	};

	// SET 6, H
	cb_instructions_[0xF4] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 6);
		return 2;
	};

	// SET 6, L
	cb_instructions_[0xF5] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 6);
		return 2;
	};

	// SET 6, (HL)
	cb_instructions_[0xF6] = [this]() -> MachineCycles {
		mmu_->SetBit<6>(registers_.hl);
		return 4;
	};

	// SET 6, A
	cb_instructions_[0xF7] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 6);
		return 2;
	};

	// SET 7, B
	cb_instructions_[0xF8] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() |= (1 << 7);
		return 2;
	};

	// SET 7, C
	cb_instructions_[0xF9] = [this]() -> MachineCycles {
		registers_.bc.GetLow() |= (1 << 7);
		return 2;
	};

	// SET 7, D
	cb_instructions_[0xFA] = [this]() -> MachineCycles {
		registers_.de.GetHigh() |= (1 << 7);
		return 2;
	};

	// SET 7, E
	cb_instructions_[0xFB] = [this]() -> MachineCycles {
		registers_.de.GetLow() |= (1 << 7);
		return 2;
	};

	// SET 7, H
	cb_instructions_[0xFC] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() |= (1 << 7);
		return 2;
	};

	// SET 7, L
	cb_instructions_[0xFD] = [this]() -> MachineCycles {
		registers_.hl.GetLow() |= (1 << 7);
		return 2;
	};

	// SET 7, (HL)
	cb_instructions_[0xFE] = [this]() -> MachineCycles {
		mmu_->SetBit<7>(registers_.hl);
		return 4;
	};

	// SET 7, A
	cb_instructions_[0xFF] = [this]() -> MachineCycles {
		registers_.af.GetHigh() |= (1 << 7);
		return 2;
	};
#pragma endregion
}

void CPU::PopulateCbInstructionNames()
{
	cb_instruction_names_[0x00] = "RLC B";
	cb_instruction_names_[0x01] = "RLC C";
	cb_instruction_names_[0x02] = "RLC D";
	cb_instruction_names_[0x03] = "RLC E";
	cb_instruction_names_[0x04] = "RLC H";
	cb_instruction_names_[0x05] = "RLC L";
	cb_instruction_names_[0x06] = "RLC (HL)";
	cb_instruction_names_[0x07] = "RLC A";
	cb_instruction_names_[0x08] = "RRC B";
	cb_instruction_names_[0x09] = "RRC C";
	cb_instruction_names_[0x0A] = "RRC D";
	cb_instruction_names_[0x0B] = "RRC E";
	cb_instruction_names_[0x0C] = "RRC H";
	cb_instruction_names_[0x0D] = "RRC L";
	cb_instruction_names_[0x0E] = "RRC (HL)";
	cb_instruction_names_[0x0F] = "RRC A";

	cb_instruction_names_[0x10] = "RL B";
	cb_instruction_names_[0x11] = "RL C";
	cb_instruction_names_[0x12] = "RL D";
	cb_instruction_names_[0x13] = "RL E";
	cb_instruction_names_[0x14] = "RL H";
	cb_instruction_names_[0x15] = "RL L";
	cb_instruction_names_[0x16] = "RL (HL)";
	cb_instruction_names_[0x17] = "RL A";
	cb_instruction_names_[0x18] = "RR B";
	cb_instruction_names_[0x19] = "RR C";
	cb_instruction_names_[0x1A] = "RR D";
	cb_instruction_names_[0x1B] = "RR E";
	cb_instruction_names_[0x1C] = "RR H";
	cb_instruction_names_[0x1D] = "RR L";
	cb_instruction_names_[0x1E] = "RR (HL)";
	cb_instruction_names_[0x1F] = "RR A";

	cb_instruction_names_[0x20] = "SLA B";
	cb_instruction_names_[0x21] = "SLA C";
	cb_instruction_names_[0x22] = "SLA D";
	cb_instruction_names_[0x23] = "SLA E";
	cb_instruction_names_[0x24] = "SLA H";
	cb_instruction_names_[0x25] = "SLA L";
	cb_instruction_names_[0x26] = "SLA (HL)";
	cb_instruction_names_[0x27] = "SLA A";
	cb_instruction_names_[0x28] = "SRA B";
	cb_instruction_names_[0x29] = "SRA C";
	cb_instruction_names_[0x2A] = "SRA D";
	cb_instruction_names_[0x2B] = "SRA E";
	cb_instruction_names_[0x2C] = "SRA H";
	cb_instruction_names_[0x2D] = "SRA L";
	cb_instruction_names_[0x2E] = "SRA (HL)";
	cb_instruction_names_[0x2F] = "SRA A";

	cb_instruction_names_[0x30] = "SWAP B";
	cb_instruction_names_[0x31] = "SWAP C";
	cb_instruction_names_[0x32] = "SWAP D";
	cb_instruction_names_[0x33] = "SWAP E";
	cb_instruction_names_[0x34] = "SWAP H";
	cb_instruction_names_[0x35] = "SWAP L";
	cb_instruction_names_[0x36] = "SWAP (HL)";
	cb_instruction_names_[0x37] = "SWAP A";
	cb_instruction_names_[0x38] = "SRL B";
	cb_instruction_names_[0x39] = "SRL C";
	cb_instruction_names_[0x3A] = "SRL D";
	cb_instruction_names_[0x3B] = "SRL E";
	cb_instruction_names_[0x3C] = "SRL H";
	cb_instruction_names_[0x3D] = "SRL L";
	cb_instruction_names_[0x3E] = "SRL (HL)";
	cb_instruction_names_[0x3F] = "SRL A";

	cb_instruction_names_[0x40] = "BIT 0, B";
	cb_instruction_names_[0x41] = "BIT 0, C";
	cb_instruction_names_[0x42] = "BIT 0, D";
	cb_instruction_names_[0x43] = "BIT 0, E";
	cb_instruction_names_[0x44] = "BIT 0, H";
	cb_instruction_names_[0x45] = "BIT 0, L";
	cb_instruction_names_[0x46] = "BIT 0, (HL)";
	cb_instruction_names_[0x47] = "BIT 0, A";
	cb_instruction_names_[0x48] = "BIT 1, B";
	cb_instruction_names_[0x49] = "BIT 1, C";
	cb_instruction_names_[0x4A] = "BIT 1, D";
	cb_instruction_names_[0x4B] = "BIT 1, E";
	cb_instruction_names_[0x4C] = "BIT 1, H";
	cb_instruction_names_[0x4D] = "BIT 1, L";
	cb_instruction_names_[0x4E] = "BIT 1, (HL)";
	cb_instruction_names_[0x4F] = "BIT 1, A";

	cb_instruction_names_[0x50] = "BIT 2, B";
	cb_instruction_names_[0x51] = "BIT 2, C";
	cb_instruction_names_[0x52] = "BIT 2, D";
	cb_instruction_names_[0x53] = "BIT 2, E";
	cb_instruction_names_[0x54] = "BIT 2, H";
	cb_instruction_names_[0x55] = "BIT 2, L";
	cb_instruction_names_[0x56] = "BIT 2, (HL)";
	cb_instruction_names_[0x57] = "BIT 2, A";
	cb_instruction_names_[0x58] = "BIT 3, B";
	cb_instruction_names_[0x59] = "BIT 3, C";
	cb_instruction_names_[0x5A] = "BIT 3, D";
	cb_instruction_names_[0x5B] = "BIT 3, E";
	cb_instruction_names_[0x5C] = "BIT 3, H";
	cb_instruction_names_[0x5D] = "BIT 3, L";
	cb_instruction_names_[0x5E] = "BIT 3, (HL)";
	cb_instruction_names_[0x5F] = "BIT 3, A";

	cb_instruction_names_[0x60] = "BIT 4, B";
	cb_instruction_names_[0x61] = "BIT 4, C";
	cb_instruction_names_[0x62] = "BIT 4, D";
	cb_instruction_names_[0x63] = "BIT 4, E";
	cb_instruction_names_[0x64] = "BIT 4, H";
	cb_instruction_names_[0x65] = "BIT 4, L";
	cb_instruction_names_[0x66] = "BIT 4, (HL)";
	cb_instruction_names_[0x67] = "BIT 4, A";
	cb_instruction_names_[0x68] = "BIT 5, B";
	cb_instruction_names_[0x69] = "BIT 5, C";
	cb_instruction_names_[0x6A] = "BIT 5, D";
	cb_instruction_names_[0x6B] = "BIT 5, E";
	cb_instruction_names_[0x6C] = "BIT 5, H";
	cb_instruction_names_[0x6D] = "BIT 5, L";
	cb_instruction_names_[0x6E] = "BIT 5, (HL)";
	cb_instruction_names_[0x6F] = "BIT 5, A";

	cb_instruction_names_[0x70] = "BIT 6, B";
	cb_instruction_names_[0x71] = "BIT 6, C";
	cb_instruction_names_[0x72] = "BIT 6, D";
	cb_instruction_names_[0x73] = "BIT 6, E";
	cb_instruction_names_[0x74] = "BIT 6, H";
	cb_instruction_names_[0x75] = "BIT 6, L";
	cb_instruction_names_[0x76] = "BIT 6, (HL)";
	cb_instruction_names_[0x77] = "BIT 6, A";
	cb_instruction_names_[0x78] = "BIT 7, B";
	cb_instruction_names_[0x79] = "BIT 7, C";
	cb_instruction_names_[0x7A] = "BIT 7, D";
	cb_instruction_names_[0x7B] = "BIT 7, E";
	cb_instruction_names_[0x7C] = "BIT 7, H";
	cb_instruction_names_[0x7D] = "BIT 7, L";
	cb_instruction_names_[0x7E] = "BIT 7, (HL)";
	cb_instruction_names_[0x7F] = "BIT 7, A";

	cb_instruction_names_[0x80] = "RES 0, B";
	cb_instruction_names_[0x81] = "RES 0, C";
	cb_instruction_names_[0x82] = "RES 0, D";
	cb_instruction_names_[0x83] = "RES 0, E";
	cb_instruction_names_[0x84] = "RES 0, H";
	cb_instruction_names_[0x85] = "RES 0, L";
	cb_instruction_names_[0x86] = "RES 0, (HL)";
	cb_instruction_names_[0x87] = "RES 0, A";
	cb_instruction_names_[0x88] = "RES 1, B";
	cb_instruction_names_[0x89] = "RES 1, C";
	cb_instruction_names_[0x8A] = "RES 1, D";
	cb_instruction_names_[0x8B] = "RES 1, E";
	cb_instruction_names_[0x8C] = "RES 1, H";
	cb_instruction_names_[0x8D] = "RES 1, L";
	cb_instruction_names_[0x8E] = "RES 1, (HL)";
	cb_instruction_names_[0x8F] = "RES 1, A";

	cb_instruction_names_[0x90] = "RES 2, B";
	cb_instruction_names_[0x91] = "RES 2, C";
	cb_instruction_names_[0x92] = "RES 2, D";
	cb_instruction_names_[0x93] = "RES 2, E";
	cb_instruction_names_[0x94] = "RES 2, H";
	cb_instruction_names_[0x95] = "RES 2, L";
	cb_instruction_names_[0x96] = "RES 2, (HL)";
	cb_instruction_names_[0x97] = "RES 2, A";
	cb_instruction_names_[0x98] = "RES 3, B";
	cb_instruction_names_[0x99] = "RES 3, C";
	cb_instruction_names_[0x9A] = "RES 3, D";
	cb_instruction_names_[0x9B] = "RES 3, E";
	cb_instruction_names_[0x9C] = "RES 3, H";
	cb_instruction_names_[0x9D] = "RES 3, L";
	cb_instruction_names_[0x9E] = "RES 3, (HL)";
	cb_instruction_names_[0x9F] = "RES 3, A";

	cb_instruction_names_[0xA0] = "RES 4, B";
	cb_instruction_names_[0xA1] = "RES 4, C";
	cb_instruction_names_[0xA2] = "RES 4, D";
	cb_instruction_names_[0xA3] = "RES 4, E";
	cb_instruction_names_[0xA4] = "RES 4, H";
	cb_instruction_names_[0xA5] = "RES 4, L";
	cb_instruction_names_[0xA6] = "RES 4, (HL)";
	cb_instruction_names_[0xA7] = "RES 4, A";
	cb_instruction_names_[0xA8] = "RES 5, B";
	cb_instruction_names_[0xA9] = "RES 5, C";
	cb_instruction_names_[0xAA] = "RES 5, D";
	cb_instruction_names_[0xAB] = "RES 5, E";
	cb_instruction_names_[0xAC] = "RES 5, H";
	cb_instruction_names_[0xAD] = "RES 5, L";
	cb_instruction_names_[0xAE] = "RES 5, (HL)";
	cb_instruction_names_[0xAF] = "RES 5, A";

	cb_instruction_names_[0xB0] = "RES 6, B";
	cb_instruction_names_[0xB1] = "RES 6, C";
	cb_instruction_names_[0xB2] = "RES 6, D";
	cb_instruction_names_[0xB3] = "RES 6, E";
	cb_instruction_names_[0xB4] = "RES 6, H";
	cb_instruction_names_[0xB5] = "RES 6, L";
	cb_instruction_names_[0xB6] = "RES 6, (HL)";
	cb_instruction_names_[0xB7] = "RES 6, A";
	cb_instruction_names_[0xB8] = "RES 7, B";
	cb_instruction_names_[0xB9] = "RES 7, C";
	cb_instruction_names_[0xBA] = "RES 7, D";
	cb_instruction_names_[0xBB] = "RES 7, E";
	cb_instruction_names_[0xBC] = "RES 7, H";
	cb_instruction_names_[0xBD] = "RES 7, L";
	cb_instruction_names_[0xBE] = "RES 7, (HL)";
	cb_instruction_names_[0xBF] = "RES 7, A";

	cb_instruction_names_[0xC0] = "SET 0, B";
	cb_instruction_names_[0xC1] = "SET 0, C";
	cb_instruction_names_[0xC2] = "SET 0, D";
	cb_instruction_names_[0xC3] = "SET 0, E";
	cb_instruction_names_[0xC4] = "SET 0, H";
	cb_instruction_names_[0xC5] = "SET 0, L";
	cb_instruction_names_[0xC6] = "SET 0, (HL)";
	cb_instruction_names_[0xC7] = "SET 0, A";
	cb_instruction_names_[0xC8] = "SET 1, B";
	cb_instruction_names_[0xC9] = "SET 1, C";
	cb_instruction_names_[0xCA] = "SET 1, D";
	cb_instruction_names_[0xCB] = "SET 1, E";
	cb_instruction_names_[0xCC] = "SET 1, H";
	cb_instruction_names_[0xCD] = "SET 1, L";
	cb_instruction_names_[0xCE] = "SET 1, (HL)";
	cb_instruction_names_[0xCF] = "SET 1, A";

	cb_instruction_names_[0xD0] = "SET 2, B";
	cb_instruction_names_[0xD1] = "SET 2, C";
	cb_instruction_names_[0xD2] = "SET 2, D";
	cb_instruction_names_[0xD3] = "SET 2, E";
	cb_instruction_names_[0xD4] = "SET 2, H";
	cb_instruction_names_[0xD5] = "SET 2, L";
	cb_instruction_names_[0xD6] = "SET 2, (HL)";
	cb_instruction_names_[0xD7] = "SET 2, A";
	cb_instruction_names_[0xD8] = "SET 3, B";
	cb_instruction_names_[0xD9] = "SET 3, C";
	cb_instruction_names_[0xDA] = "SET 3, D";
	cb_instruction_names_[0xDB] = "SET 3, E";
	cb_instruction_names_[0xDC] = "SET 3, H";
	cb_instruction_names_[0xDD] = "SET 3, L";
	cb_instruction_names_[0xDE] = "SET 3, (HL)";
	cb_instruction_names_[0xDF] = "SET 3, A";

	cb_instruction_names_[0xE0] = "SET 4, B";
	cb_instruction_names_[0xE1] = "SET 4, C";
	cb_instruction_names_[0xE2] = "SET 4, D";
	cb_instruction_names_[0xE3] = "SET 4, E";
	cb_instruction_names_[0xE4] = "SET 4, H";
	cb_instruction_names_[0xE5] = "SET 4, L";
	cb_instruction_names_[0xE6] = "SET 4, (HL)";
	cb_instruction_names_[0xE7] = "SET 4, A";
	cb_instruction_names_[0xE8] = "SET 5, B";
	cb_instruction_names_[0xE9] = "SET 5, C";
	cb_instruction_names_[0xEA] = "SET 5, D";
	cb_instruction_names_[0xEB] = "SET 5, E";
	cb_instruction_names_[0xEC] = "SET 5, H";
	cb_instruction_names_[0xED] = "SET 5, L";
	cb_instruction_names_[0xEE] = "SET 5, (HL)";
	cb_instruction_names_[0xEF] = "SET 5, A";

	cb_instruction_names_[0xF0] = "SET 6, B";
	cb_instruction_names_[0xF1] = "SET 6, C";
	cb_instruction_names_[0xF2] = "SET 6, D";
	cb_instruction_names_[0xF3] = "SET 6, E";
	cb_instruction_names_[0xF4] = "SET 6, H";
	cb_instruction_names_[0xF5] = "SET 6, L";
	cb_instruction_names_[0xF6] = "SET 6, (HL)";
	cb_instruction_names_[0xF7] = "SET 6, A";
	cb_instruction_names_[0xF8] = "SET 7, B";
	cb_instruction_names_[0xF9] = "SET 7, C";
	cb_instruction_names_[0xFA] = "SET 7, D";
	cb_instruction_names_[0xFB] = "SET 7, E";
	cb_instruction_names_[0xFC] = "SET 7, H";
	cb_instruction_names_[0xFD] = "SET 7, L";
	cb_instruction_names_[0xFE] = "SET 7, (HL)";
	cb_instruction_names_[0xFF] = "SET 7, A";
}
