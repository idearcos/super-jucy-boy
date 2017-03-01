#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"
#include "InstructionMnemonics.h"

void CPU::PopulateCbInstructions()
{
	for (int ii = 0; ii < instructions_.size(); ++ii)
	{
		cb_instructions_[ii] = [this, ii]() {
			registers_.pc = previous_pc_;
			std::stringstream error;
			error << "CB Instruction not implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << GetCbInstructionMnemonic(ii) << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc << ")";
			throw std::runtime_error(error.str());
		};
	}

#pragma region CB instructions 0x00 - 0x0F
	// RLC B
	cb_instructions_[0x00] = [this]() {
		Rlc(registers_.bc.GetHigh());
	};

	// RLC C
	cb_instructions_[0x01] = [this]() {
		Rlc(registers_.bc.GetLow());
	};

	// RLC D
	cb_instructions_[0x02] = [this]() {
		Rlc(registers_.de.GetHigh());
	};

	// RLC E
	cb_instructions_[0x03] = [this]() {
		Rlc(registers_.de.GetLow());
	};

	// RLC H
	cb_instructions_[0x04] = [this]() {
		Rlc(registers_.hl.GetHigh());
	};

	// RLC L
	cb_instructions_[0x05] = [this]() {
		Rlc(registers_.hl.GetLow());
	};

	// RLC (HL)
	cb_instructions_[0x06] = [this]() {
		auto value = ReadByte(registers_.hl);
		Rlc(value);
		WriteByte(registers_.hl, value);
	};

	// RLC A
	cb_instructions_[0x07] = [this]() {
		Rlc(registers_.af.GetHigh());
	};

	// RRC B
	cb_instructions_[0x08] = [this]() {
		Rrc(registers_.bc.GetHigh());
	};

	// RRC C
	cb_instructions_[0x09] = [this]() {
		Rrc(registers_.bc.GetLow());
	};

	// RRC D
	cb_instructions_[0x0A] = [this]() {
		Rrc(registers_.de.GetHigh());
	};

	// RRC E
	cb_instructions_[0x0B] = [this]() {
		Rrc(registers_.de.GetLow());
	};

	// RRC H
	cb_instructions_[0x0C] = [this]() {
		Rrc(registers_.hl.GetHigh());
	};

	// RRC L
	cb_instructions_[0x0D] = [this]() {
		Rrc(registers_.hl.GetLow());
	};

	// RRC (HL)
	cb_instructions_[0x0E] = [this]() {
		auto value = ReadByte(registers_.hl);
		Rrc(value);
		WriteByte(registers_.hl, value);
	};

	// RRC A
	cb_instructions_[0x0F] = [this]() {
		Rrc(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region CB instructions 0x10 - 0x1F
	// RL B
	cb_instructions_[0x10] = [this]() {
		Rl(registers_.bc.GetHigh());
	};

	// RL C
	cb_instructions_[0x11] = [this]() {
		Rl(registers_.bc.GetLow());
	};

	// RL D
	cb_instructions_[0x12] = [this]() {
		Rl(registers_.de.GetHigh());
	};

	// RL E
	cb_instructions_[0x13] = [this]() {
		Rl(registers_.de.GetLow());
	};

	// RL H
	cb_instructions_[0x14] = [this]() {
		Rl(registers_.hl.GetHigh());
	};

	// RL L
	cb_instructions_[0x15] = [this]() {
		Rl(registers_.hl.GetLow());
	};

	// RL (HL)
	cb_instructions_[0x16] = [this]() {
		auto value = ReadByte(registers_.hl);
		Rl(value);
		WriteByte(registers_.hl, value);
	};

	// RL A
	cb_instructions_[0x17] = [this]() {
		Rl(registers_.af.GetHigh());
	};

	// RR B
	cb_instructions_[0x18] = [this]() {
		Rr(registers_.bc.GetHigh());
	};

	// RR C
	cb_instructions_[0x19] = [this]() {
		Rr(registers_.bc.GetLow());
	};

	// RR D
	cb_instructions_[0x1A] = [this]() {
		Rr(registers_.de.GetHigh());
	};

	// RR E
	cb_instructions_[0x1B] = [this]() {
		Rr(registers_.de.GetLow());
	};

	// RR H
	cb_instructions_[0x1C] = [this]() {
		Rr(registers_.hl.GetHigh());
	};

	// RR L
	cb_instructions_[0x1D] = [this]() {
		Rr(registers_.hl.GetLow());
	};

	// RR (HL)
	cb_instructions_[0x1E] = [this]() {
		auto value = ReadByte(registers_.hl);
		Rr(value);
		WriteByte(registers_.hl, value);
	};

	// RR A
	cb_instructions_[0x1F] = [this]() {
		Rr(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region CB instructions 0x20 - 0x2F
	// SLA B
	cb_instructions_[0x20] = [this]() {
		Sla(registers_.bc.GetHigh());
	};

	// SLA C
	cb_instructions_[0x21] = [this]() {
		Sla(registers_.bc.GetLow());
	};

	// SLA D
	cb_instructions_[0x22] = [this]() {
		Sla(registers_.de.GetHigh());
	};

	// SLA E
	cb_instructions_[0x23] = [this]() {
		Sla(registers_.de.GetLow());
	};

	// SLA H
	cb_instructions_[0x24] = [this]() {
		Sla(registers_.hl.GetHigh());
	};

	// SLA L
	cb_instructions_[0x25] = [this]() {
		Sla(registers_.hl.GetLow());
	};

	// SLA (HL)
	cb_instructions_[0x26] = [this]() {
		auto value = ReadByte(registers_.hl);
		Sla(value);
		WriteByte(registers_.hl, value);
	};

	// SLA A
	cb_instructions_[0x27] = [this]() {
		Sla(registers_.af.GetHigh());
	};

	// SRA B
	cb_instructions_[0x28] = [this]() {
		Sra(registers_.bc.GetHigh());
	};

	// SRA C
	cb_instructions_[0x29] = [this]() {
		Sra(registers_.bc.GetLow());
	};

	// SRA D
	cb_instructions_[0x2A] = [this]() {
		Sra(registers_.de.GetHigh());
	};

	// SRA E
	cb_instructions_[0x2B] = [this]() {
		Sra(registers_.de.GetLow());
	};

	// SRA H
	cb_instructions_[0x2C] = [this]() {
		Sra(registers_.hl.GetHigh());
	};

	// SRA L
	cb_instructions_[0x2D] = [this]() {
		Sra(registers_.hl.GetLow());
	};

	// SRA (HL)
	cb_instructions_[0x2E] = [this]() {
		auto value = ReadByte(registers_.hl);
		Sra(value);
		WriteByte(registers_.hl, value);
	};

	// SRA A
	cb_instructions_[0x2F] = [this]() {
		Sra(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region CB instructions 0x30 - 0x3F
	// SWAP B
	cb_instructions_[0x30] = [this]() {
		Swap(registers_.bc.GetHigh());
	};

	// SWAP C
	cb_instructions_[0x31] = [this]() {
		Swap(registers_.bc.GetLow());
	};

	// SWAP D
	cb_instructions_[0x32] = [this]() {
		Swap(registers_.de.GetHigh());
	};

	// SWAP E
	cb_instructions_[0x33] = [this]() {
		Swap(registers_.de.GetLow());
	};

	// SWAP H
	cb_instructions_[0x34] = [this]() {
		Swap(registers_.hl.GetHigh());
	};

	// SWAP L
	cb_instructions_[0x35] = [this]() {
		Swap(registers_.hl.GetLow());
	};

	// SWAP (HL)
	cb_instructions_[0x36] = [this]() {
		auto value = ReadByte(registers_.hl);
		Swap(value);
		WriteByte(registers_.hl, value);
	};

	// SWAP A
	cb_instructions_[0x37] = [this]() {
		Swap(registers_.af.GetHigh());
	};

	// SRL B
	cb_instructions_[0x38] = [this]() {
		Srl(registers_.bc.GetHigh());
	};

	// SRL C
	cb_instructions_[0x39] = [this]() {
		Srl(registers_.bc.GetLow());
	};

	// SRL D
	cb_instructions_[0x3A] = [this]() {
		Srl(registers_.de.GetHigh());
	};

	// SRL E
	cb_instructions_[0x3B] = [this]() {
		Srl(registers_.de.GetLow());
	};

	// SRL H
	cb_instructions_[0x3C] = [this]() {
		Srl(registers_.hl.GetHigh());
	};

	// SRL L
	cb_instructions_[0x3D] = [this]() {
		Srl(registers_.hl.GetLow());
	};

	// SRL (HL)
	cb_instructions_[0x3E] = [this]() {
		auto value = ReadByte(registers_.hl);
		Srl(value);
		WriteByte(registers_.hl, value);
	};

	// SRL A
	cb_instructions_[0x3F] = [this]() {
		Srl(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region CB instructions 0x40 - 0x4F
	// BIT 0, B
	cb_instructions_[0x40] = [this]() {
		TestBit(registers_.bc.GetHigh(), 0);
	};

	// BIT 0, C
	cb_instructions_[0x41] = [this]() {
		TestBit(registers_.bc.GetLow(), 0);
	};

	// BIT 0, D
	cb_instructions_[0x42] = [this]() {
		TestBit(registers_.de.GetHigh(), 0);
	};

	// BIT 0, E
	cb_instructions_[0x43] = [this]() {
		TestBit(registers_.de.GetLow(), 0);
	};

	// BIT 0, H
	cb_instructions_[0x44] = [this]() {
		TestBit(registers_.hl.GetHigh(), 0);
	};

	// BIT 0, L
	cb_instructions_[0x45] = [this]() {
		TestBit(registers_.hl.GetLow(), 0);
	};

	// BIT 0, (HL)
	cb_instructions_[0x46] = [this]() {
		TestBit(ReadByte(registers_.hl), 0);
	};

	// BIT 0, A
	cb_instructions_[0x47] = [this]() {
		TestBit(registers_.af.GetHigh(), 0);
	};

	// BIT 1, B
	cb_instructions_[0x48] = [this]() {
		TestBit(registers_.bc.GetHigh(), 1);
	};

	// BIT 1, C
	cb_instructions_[0x49] = [this]() {
		TestBit(registers_.bc.GetLow(), 1);
	};

	// BIT 1, D
	cb_instructions_[0x4A] = [this]() {
		TestBit(registers_.de.GetHigh(), 1);
	};

	// BIT 1, E
	cb_instructions_[0x4B] = [this]() {
		TestBit(registers_.de.GetLow(), 1);
	};

	// BIT 1, H
	cb_instructions_[0x4C] = [this]() {
		TestBit(registers_.hl.GetHigh(), 1);
	};

	// BIT 1, L
	cb_instructions_[0x4D] = [this]() {
		TestBit(registers_.hl.GetLow(), 1);
	};

	// BIT 1, (HL)
	cb_instructions_[0x4E] = [this]() {
		TestBit(ReadByte(registers_.hl), 1);
	};

	// BIT 1, A
	cb_instructions_[0x4F] = [this]() {
		TestBit(registers_.af.GetHigh(), 1);
	};
#pragma endregion

#pragma region CB instructions 0x50 - 0x5F
	// BIT 2, B
	cb_instructions_[0x50] = [this]() {
		TestBit(registers_.bc.GetHigh(), 2);
	};

	// BIT 2, C
	cb_instructions_[0x51] = [this]() {
		TestBit(registers_.bc.GetLow(), 2);
	};

	// BIT 2, D
	cb_instructions_[0x52] = [this]() {
		TestBit(registers_.de.GetHigh(), 2);
	};

	// BIT 2, E
	cb_instructions_[0x53] = [this]() {
		TestBit(registers_.de.GetLow(), 2);
	};

	// BIT 2, H
	cb_instructions_[0x54] = [this]() {
		TestBit(registers_.hl.GetHigh(), 2);
	};

	// BIT 2, L
	cb_instructions_[0x55] = [this]() {
		TestBit(registers_.hl.GetLow(), 2);
	};

	// BIT 2, (HL)
	cb_instructions_[0x56] = [this]() {
		TestBit(ReadByte(registers_.hl), 2);
	};

	// BIT 2, A
	cb_instructions_[0x57] = [this]() {
		TestBit(registers_.af.GetHigh(), 2);
	};

	// BIT 3, B
	cb_instructions_[0x58] = [this]() {
		TestBit(registers_.bc.GetHigh(), 3);
	};

	// BIT 3, C
	cb_instructions_[0x59] = [this]() {
		TestBit(registers_.bc.GetLow(), 3);
	};

	// BIT 3, D
	cb_instructions_[0x5A] = [this]() {
		TestBit(registers_.de.GetHigh(), 3);
	};

	// BIT 3, E
	cb_instructions_[0x5B] = [this]() {
		TestBit(registers_.de.GetLow(), 3);
	};

	// BIT 3, H
	cb_instructions_[0x5C] = [this]() {
		TestBit(registers_.hl.GetHigh(), 3);
	};

	// BIT 3, L
	cb_instructions_[0x5D] = [this]() {
		TestBit(registers_.hl.GetLow(), 3);
	};

	// BIT 3, (HL)
	cb_instructions_[0x5E] = [this]() {
		TestBit(ReadByte(registers_.hl), 3);
	};

	// BIT 3, A
	cb_instructions_[0x5F] = [this]() {
		TestBit(registers_.af.GetHigh(), 3);
	};
#pragma endregion

#pragma region CB instructions 0x60 - 0x6F
	// BIT 4, B
	cb_instructions_[0x60] = [this]() {
		TestBit(registers_.bc.GetHigh(), 4);
	};

	// BIT 4, C
	cb_instructions_[0x61] = [this]() {
		TestBit(registers_.bc.GetLow(), 4);
	};

	// BIT 4, D
	cb_instructions_[0x62] = [this]() {
		TestBit(registers_.de.GetHigh(), 4);
	};

	// BIT 4, E
	cb_instructions_[0x63] = [this]() {
		TestBit(registers_.de.GetLow(), 4);
	};

	// BIT 4, H
	cb_instructions_[0x64] = [this]() {
		TestBit(registers_.hl.GetHigh(), 4);
	};

	// BIT 4, L
	cb_instructions_[0x65] = [this]() {
		TestBit(registers_.hl.GetLow(), 4);
	};

	// BIT 4, (HL)
	cb_instructions_[0x66] = [this]() {
		TestBit(ReadByte(registers_.hl), 4);
	};

	// BIT 4, A
	cb_instructions_[0x67] = [this]() {
		TestBit(registers_.af.GetHigh(), 4);
	};

	// BIT 5, B
	cb_instructions_[0x68] = [this]() {
		TestBit(registers_.bc.GetHigh(), 5);
	};

	// BIT 5, C
	cb_instructions_[0x69] = [this]() {
		TestBit(registers_.bc.GetLow(), 5);
	};

	// BIT 5, D
	cb_instructions_[0x6A] = [this]() {
		TestBit(registers_.de.GetHigh(), 5);
	};

	// BIT 5, E
	cb_instructions_[0x6B] = [this]() {
		TestBit(registers_.de.GetLow(), 5);
	};

	// BIT 5, H
	cb_instructions_[0x6C] = [this]() {
		TestBit(registers_.hl.GetHigh(), 5);
	};

	// BIT 5, L
	cb_instructions_[0x6D] = [this]() {
		TestBit(registers_.hl.GetLow(), 5);
	};

	// BIT 5, (HL)
	cb_instructions_[0x6E] = [this]() {
		TestBit(ReadByte(registers_.hl), 5);
	};

	// BIT 5, A
	cb_instructions_[0x6F] = [this]() {
		TestBit(registers_.af.GetHigh(), 5);
	};
#pragma endregion

#pragma region CB instructions 0x70 - 0x7F
	// BIT 6, B
	cb_instructions_[0x70] = [this]() {
		TestBit(registers_.bc.GetHigh(), 6);
	};

	// BIT 6, C
	cb_instructions_[0x71] = [this]() {
		TestBit(registers_.bc.GetLow(), 6);
	};

	// BIT 6, D
	cb_instructions_[0x72] = [this]() {
		TestBit(registers_.de.GetHigh(), 6);
	};

	// BIT 6, E
	cb_instructions_[0x73] = [this]() {
		TestBit(registers_.de.GetLow(), 6);
	};

	// BIT 6, H
	cb_instructions_[0x74] = [this]() {
		TestBit(registers_.hl.GetHigh(), 6);
	};

	// BIT 6, L
	cb_instructions_[0x75] = [this]() {
		TestBit(registers_.hl.GetLow(), 6);
	};

	// BIT 6, (HL)
	cb_instructions_[0x76] = [this]() {
		TestBit(ReadByte(registers_.hl), 6);
	};

	// BIT 6, A
	cb_instructions_[0x77] = [this]() {
		TestBit(registers_.af.GetHigh(), 6);
	};

	// BIT 7, B
	cb_instructions_[0x78] = [this]() {
		TestBit(registers_.bc.GetHigh(), 7);
	};

	// BIT 7, C
	cb_instructions_[0x79] = [this]() {
		TestBit(registers_.bc.GetLow(), 7);
	};

	// BIT 7, D
	cb_instructions_[0x7A] = [this]() {
		TestBit(registers_.de.GetHigh(), 7);
	};

	// BIT 7, E
	cb_instructions_[0x7B] = [this]() {
		TestBit(registers_.de.GetLow(), 7);
	};

	// BIT 7, H
	cb_instructions_[0x7C] = [this]() {
		TestBit(registers_.hl.GetHigh(), 7);
	};

	// BIT 7, L
	cb_instructions_[0x7D] = [this]() {
		TestBit(registers_.hl.GetLow(), 7);
	};

	// BIT 7, (HL)
	cb_instructions_[0x7E] = [this]() {
		TestBit(ReadByte(registers_.hl), 7);
	};

	// BIT 7, A
	cb_instructions_[0x7F] = [this]() {
		TestBit(registers_.af.GetHigh(), 7);
	};
#pragma endregion

#pragma region CB instructions 0x80 - 0x8F
	// RES 0, B
	cb_instructions_[0x80] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 0);
	};

	// RES 0, C
	cb_instructions_[0x81] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 0);
	};

	// RES 0, D
	cb_instructions_[0x82] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 0);
	};

	// RES 0, E
	cb_instructions_[0x83] = [this]() {
		registers_.de.GetLow() &= ~(1 << 0);
	};

	// RES 0, H
	cb_instructions_[0x84] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 0);
	};

	// RES 0, L
	cb_instructions_[0x85] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 0);
	};

	// RES 0, (HL)
	cb_instructions_[0x86] = [this]() {
		WriteByte(registers_.hl, ~(1 << 0) & ReadByte(registers_.hl));
	};

	// RES 0, A
	cb_instructions_[0x87] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 0);
	};

	// RES 1, B
	cb_instructions_[0x88] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 1);
	};

	// RES 1, C
	cb_instructions_[0x89] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 1);
	};

	// RES 1, D
	cb_instructions_[0x8A] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 1);
	};

	// RES 1, E
	cb_instructions_[0x8B] = [this]() {
		registers_.de.GetLow() &= ~(1 << 1);
	};

	// RES 1, H
	cb_instructions_[0x8C] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 1);
	};

	// RES 1, L
	cb_instructions_[0x8D] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 1);
	};

	// RES 1, (HL)
	cb_instructions_[0x8E] = [this]() {
		WriteByte(registers_.hl, ~(1 << 1) & ReadByte(registers_.hl));
	};

	// RES 1, A
	cb_instructions_[0x8F] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 1);
	};
#pragma endregion

#pragma region CB instructions 0x90 - 0x9F
	// RES 2, B
	cb_instructions_[0x90] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 2);
	};

	// RES 2, C
	cb_instructions_[0x91] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 2);
	};

	// RES 2, D
	cb_instructions_[0x92] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 2);
	};

	// RES 2, E
	cb_instructions_[0x93] = [this]() {
		registers_.de.GetLow() &= ~(1 << 2);
	};

	// RES 2, H
	cb_instructions_[0x94] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 2);
	};

	// RES 2, L
	cb_instructions_[0x95] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 2);
	};

	// RES 2, (HL)
	cb_instructions_[0x96] = [this]() {
		WriteByte(registers_.hl, ~(1 << 2) & ReadByte(registers_.hl));
	};

	// RES 2, A
	cb_instructions_[0x97] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 2);
	};

	// RES 3, B
	cb_instructions_[0x98] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 3);
	};

	// RES 3, C
	cb_instructions_[0x99] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 3);
	};

	// RES 3, D
	cb_instructions_[0x9A] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 3);
	};

	// RES 3, E
	cb_instructions_[0x9B] = [this]() {
		registers_.de.GetLow() &= ~(1 << 3);
	};

	// RES 3, H
	cb_instructions_[0x9C] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 3);
	};

	// RES 3, L
	cb_instructions_[0x9D] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 3);
	};

	// RES 3, (HL)
	cb_instructions_[0x9E] = [this]() {
		WriteByte(registers_.hl, ~(1 << 3) & ReadByte(registers_.hl));
	};

	// RES 3, A
	cb_instructions_[0x9F] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 3);
	};
#pragma endregion

#pragma region CB instructions 0xA0 - 0xAF
	// RES 4, B
	cb_instructions_[0xA0] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 4);
	};

	// RES 4, C
	cb_instructions_[0xA1] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 4);
	};

	// RES 4, D
	cb_instructions_[0xA2] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 4);
	};

	// RES 4, E
	cb_instructions_[0xA3] = [this]() {
		registers_.de.GetLow() &= ~(1 << 4);
	};

	// RES 4, H
	cb_instructions_[0xA4] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 4);
	};

	// RES 4, L
	cb_instructions_[0xA5] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 4);
	};

	// RES 4, (HL)
	cb_instructions_[0xA6] = [this]() {
		WriteByte(registers_.hl, ~(1 << 4) & ReadByte(registers_.hl));
	};

	// RES 4, A
	cb_instructions_[0xA7] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 4);
	};

	// RES 5, B
	cb_instructions_[0xA8] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 5);
	};

	// RES 5, C
	cb_instructions_[0xA9] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 5);
	};

	// RES 5, D
	cb_instructions_[0xAA] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 5);
	};

	// RES 5, E
	cb_instructions_[0xAB] = [this]() {
		registers_.de.GetLow() &= ~(1 << 5);
	};

	// RES 5, H
	cb_instructions_[0xAC] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 5);
	};

	// RES 5, L
	cb_instructions_[0xAD] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 5);
	};

	// RES 5, (HL)
	cb_instructions_[0xAE] = [this]() {
		WriteByte(registers_.hl, ~(1 << 5) & ReadByte(registers_.hl));
	};

	// RES 5, A
	cb_instructions_[0xAF] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 5);
	};
#pragma endregion

#pragma region CB instructions 0xB0 - 0xBF
	// RES 6, B
	cb_instructions_[0xB0] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 6);
	};

	// RES 6, C
	cb_instructions_[0xB1] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 6);
	};

	// RES 6, D
	cb_instructions_[0xB2] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 6);
	};

	// RES 6, E
	cb_instructions_[0xB3] = [this]() {
		registers_.de.GetLow() &= ~(1 << 6);
	};

	// RES 6, H
	cb_instructions_[0xB4] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 6);
	};

	// RES 6, L
	cb_instructions_[0xB5] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 6);
	};

	// RES 6, (HL)
	cb_instructions_[0xB6] = [this]() {
		WriteByte(registers_.hl, ~(1 << 6) & ReadByte(registers_.hl));
	};

	// RES 6, A
	cb_instructions_[0xB7] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 6);
	};

	// RES 7, B
	cb_instructions_[0xB8] = [this]() {
		registers_.bc.GetHigh() &= ~(1 << 7);
	};

	// RES 7, C
	cb_instructions_[0xB9] = [this]() {
		registers_.bc.GetLow() &= ~(1 << 7);
	};

	// RES 7, D
	cb_instructions_[0xBA] = [this]() {
		registers_.de.GetHigh() &= ~(1 << 7);
	};

	// RES 7, E
	cb_instructions_[0xBB] = [this]() {
		registers_.de.GetLow() &= ~(1 << 7);
	};

	// RES 7, H
	cb_instructions_[0xBC] = [this]() {
		registers_.hl.GetHigh() &= ~(1 << 7);
	};

	// RES 7, L
	cb_instructions_[0xBD] = [this]() {
		registers_.hl.GetLow() &= ~(1 << 7);
	};

	// RES 7, (HL)
	cb_instructions_[0xBE] = [this]() {
		WriteByte(registers_.hl, ~(1 << 7) & ReadByte(registers_.hl));
	};

	// RES 7, A
	cb_instructions_[0xBF] = [this]() {
		registers_.af.GetHigh() &= ~(1 << 7);
	};
#pragma endregion

#pragma region CB instructions 0xC0 - 0xCF
	// SET 0, B
	cb_instructions_[0xC0] = [this]() {
		registers_.bc.GetHigh() |= (1 << 0);
	};

	// SET 0, C
	cb_instructions_[0xC1] = [this]() {
		registers_.bc.GetLow() |= (1 << 0);
	};

	// SET 0, D
	cb_instructions_[0xC2] = [this]() {
		registers_.de.GetHigh() |= (1 << 0);
	};

	// SET 0, E
	cb_instructions_[0xC3] = [this]() {
		registers_.de.GetLow() |= (1 << 0);
	};

	// SET 0, H
	cb_instructions_[0xC4] = [this]() {
		registers_.hl.GetHigh() |= (1 << 0);
	};

	// SET 0, L
	cb_instructions_[0xC5] = [this]() {
		registers_.hl.GetLow() |= (1 << 0);
	};

	// SET 0, (HL)
	cb_instructions_[0xC6] = [this]() {
		WriteByte(registers_.hl, (1 << 0) | ReadByte(registers_.hl));
	};

	// SET 0, A
	cb_instructions_[0xC7] = [this]() {
		registers_.af.GetHigh() |= (1 << 0);
	};

	// SET 1, B
	cb_instructions_[0xC8] = [this]() {
		registers_.bc.GetHigh() |= (1 << 1);
	};

	// SET 1, C
	cb_instructions_[0xC9] = [this]() {
		registers_.bc.GetLow() |= (1 << 1);
	};

	// SET 1, D
	cb_instructions_[0xCA] = [this]() {
		registers_.de.GetHigh() |= (1 << 1);
	};

	// SET 1, E
	cb_instructions_[0xCB] = [this]() {
		registers_.de.GetLow() |= (1 << 1);
	};

	// SET 1, H
	cb_instructions_[0xCC] = [this]() {
		registers_.hl.GetHigh() |= (1 << 1);
	};

	// SET 1, L
	cb_instructions_[0xCD] = [this]() {
		registers_.hl.GetLow() |= (1 << 1);
	};

	// SET 1, (HL)
	cb_instructions_[0xCE] = [this]() {
		WriteByte(registers_.hl, (1 << 1) | ReadByte(registers_.hl));
	};

	// SET 1, A
	cb_instructions_[0xCF] = [this]() {
		registers_.af.GetHigh() |= (1 << 1);
	};
#pragma endregion

#pragma region CB instructions 0xD0 - 0xDF
	// SET 2, B
	cb_instructions_[0xD0] = [this]() {
		registers_.bc.GetHigh() |= (1 << 2);
	};

	// SET 2, C
	cb_instructions_[0xD1] = [this]() {
		registers_.bc.GetLow() |= (1 << 2);
	};

	// SET 2, D
	cb_instructions_[0xD2] = [this]() {
		registers_.de.GetHigh() |= (1 << 2);
	};

	// SET 2, E
	cb_instructions_[0xD3] = [this]() {
		registers_.de.GetLow() |= (1 << 2);
	};

	// SET 2, H
	cb_instructions_[0xD4] = [this]() {
		registers_.hl.GetHigh() |= (1 << 2);
	};

	// SET 2, L
	cb_instructions_[0xD5] = [this]() {
		registers_.hl.GetLow() |= (1 << 2);
	};

	// SET 2, (HL)
	cb_instructions_[0xD6] = [this]() {
		WriteByte(registers_.hl, (1 << 2) | ReadByte(registers_.hl));
	};

	// SET 2, A
	cb_instructions_[0xD7] = [this]() {
		registers_.af.GetHigh() |= (1 << 2);
	};

	// SET 3, B
	cb_instructions_[0xD8] = [this]() {
		registers_.bc.GetHigh() |= (1 << 3);
	};

	// SET 3, C
	cb_instructions_[0xD9] = [this]() {
		registers_.bc.GetLow() |= (1 << 3);
	};

	// SET 3, D
	cb_instructions_[0xDA] = [this]() {
		registers_.de.GetHigh() |= (1 << 3);
	};

	// SET 3, E
	cb_instructions_[0xDB] = [this]() {
		registers_.de.GetLow() |= (1 << 3);
	};

	// SET 3, H
	cb_instructions_[0xDC] = [this]() {
		registers_.hl.GetHigh() |= (1 << 3);
	};

	// SET 3, L
	cb_instructions_[0xDD] = [this]() {
		registers_.hl.GetLow() |= (1 << 3);
	};

	// SET 3, (HL)
	cb_instructions_[0xDE] = [this]() {
		WriteByte(registers_.hl, (1 << 3) | ReadByte(registers_.hl));
	};

	// SET 3, A
	cb_instructions_[0xDF] = [this]() {
		registers_.af.GetHigh() |= (1 << 3);
	};
#pragma endregion

#pragma region CB instructions 0xE0 - 0xEF
	// SET 4, B
	cb_instructions_[0xE0] = [this]() {
		registers_.bc.GetHigh() |= (1 << 4);
	};

	// SET 4, C
	cb_instructions_[0xE1] = [this]() {
		registers_.bc.GetLow() |= (1 << 4);
	};

	// SET 4, D
	cb_instructions_[0xE2] = [this]() {
		registers_.de.GetHigh() |= (1 << 4);
	};

	// SET 4, E
	cb_instructions_[0xE3] = [this]() {
		registers_.de.GetLow() |= (1 << 4);
	};

	// SET 4, H
	cb_instructions_[0xE4] = [this]() {
		registers_.hl.GetHigh() |= (1 << 4);
	};

	// SET 4, L
	cb_instructions_[0xE5] = [this]() {
		registers_.hl.GetLow() |= (1 << 4);
	};

	// SET 4, (HL)
	cb_instructions_[0xE6] = [this]() {
		WriteByte(registers_.hl, (1 << 4) | ReadByte(registers_.hl));
	};

	// SET 4, A
	cb_instructions_[0xE7] = [this]() {
		registers_.af.GetHigh() |= (1 << 4);
	};

	// SET 5, B
	cb_instructions_[0xE8] = [this]() {
		registers_.bc.GetHigh() |= (1 << 5);
	};

	// SET 5, C
	cb_instructions_[0xE9] = [this]() {
		registers_.bc.GetLow() |= (1 << 5);
	};

	// SET 5, D
	cb_instructions_[0xEA] = [this]() {
		registers_.de.GetHigh() |= (1 << 5);
	};

	// SET 5, E
	cb_instructions_[0xEB] = [this]() {
		registers_.de.GetLow() |= (1 << 5);
	};

	// SET 5, H
	cb_instructions_[0xEC] = [this]() {
		registers_.hl.GetHigh() |= (1 << 5);
	};

	// SET 5, L
	cb_instructions_[0xED] = [this]() {
		registers_.hl.GetLow() |= (1 << 5);
	};

	// SET 5, (HL)
	cb_instructions_[0xEE] = [this]() {
		WriteByte(registers_.hl, (1 << 5) | ReadByte(registers_.hl));
	};

	// SET 5, A
	cb_instructions_[0xEF] = [this]() {
		registers_.af.GetHigh() |= (1 << 5);
	};
#pragma endregion

#pragma region CB instructions 0xF0 - 0xFF
	// SET 6, B
	cb_instructions_[0xF0] = [this]() {
		registers_.bc.GetHigh() |= (1 << 6);
	};

	// SET 6, C
	cb_instructions_[0xF1] = [this]() {
		registers_.bc.GetLow() |= (1 << 6);
	};

	// SET 6, D
	cb_instructions_[0xF2] = [this]() {
		registers_.de.GetHigh() |= (1 << 6);
	};

	// SET 6, E
	cb_instructions_[0xF3] = [this]() {
		registers_.de.GetLow() |= (1 << 6);
	};

	// SET 6, H
	cb_instructions_[0xF4] = [this]() {
		registers_.hl.GetHigh() |= (1 << 6);
	};

	// SET 6, L
	cb_instructions_[0xF5] = [this]() {
		registers_.hl.GetLow() |= (1 << 6);
	};

	// SET 6, (HL)
	cb_instructions_[0xF6] = [this]() {
		WriteByte(registers_.hl, (1 << 6) | ReadByte(registers_.hl));
	};

	// SET 6, A
	cb_instructions_[0xF7] = [this]() {
		registers_.af.GetHigh() |= (1 << 6);
	};

	// SET 7, B
	cb_instructions_[0xF8] = [this]() {
		registers_.bc.GetHigh() |= (1 << 7);
	};

	// SET 7, C
	cb_instructions_[0xF9] = [this]() {
		registers_.bc.GetLow() |= (1 << 7);
	};

	// SET 7, D
	cb_instructions_[0xFA] = [this]() {
		registers_.de.GetHigh() |= (1 << 7);
	};

	// SET 7, E
	cb_instructions_[0xFB] = [this]() {
		registers_.de.GetLow() |= (1 << 7);
	};

	// SET 7, H
	cb_instructions_[0xFC] = [this]() {
		registers_.hl.GetHigh() |= (1 << 7);
	};

	// SET 7, L
	cb_instructions_[0xFD] = [this]() {
		registers_.hl.GetLow() |= (1 << 7);
	};

	// SET 7, (HL)
	cb_instructions_[0xFE] = [this]() {
		WriteByte(registers_.hl, (1 << 7) | ReadByte(registers_.hl));
	};

	// SET 7, A
	cb_instructions_[0xFF] = [this]() {
		registers_.af.GetHigh() |= (1 << 7);
	};
#pragma endregion
}
