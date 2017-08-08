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
	cb_instructions_[0x00] = [this]() { registers_.bc.High() = Rlc(registers_.bc.High()); };
	// RLC C
	cb_instructions_[0x01] = [this]() { registers_.bc.Low() = Rlc(registers_.bc.Low()); };
	// RLC D
	cb_instructions_[0x02] = [this]() { registers_.de.High() = Rlc(registers_.de.High()); };
	// RLC E
	cb_instructions_[0x03] = [this]() { registers_.de.Low() = Rlc(registers_.de.Low()); };
	// RLC H
	cb_instructions_[0x04] = [this]() { registers_.hl.High() = Rlc(registers_.hl.High()); };
	// RLC L
	cb_instructions_[0x05] = [this]() { registers_.hl.Low() = Rlc(registers_.hl.Low()); };
	// RLC (HL)
	cb_instructions_[0x06] = [this]() { WriteByte(registers_.hl, Rlc(ReadByte(registers_.hl))); };
	// RLC A
	cb_instructions_[0x07] = [this]() { registers_.af.High() = Rlc(registers_.af.High()); };
	// RRC B
	cb_instructions_[0x08] = [this]() { registers_.bc.High() = Rrc(registers_.bc.High()); };
	// RRC C
	cb_instructions_[0x09] = [this]() { registers_.bc.Low() = Rrc(registers_.bc.Low()); };
	// RRC D
	cb_instructions_[0x0A] = [this]() { registers_.de.High() = Rrc(registers_.de.High()); };
	// RRC E
	cb_instructions_[0x0B] = [this]() { registers_.de.Low() = Rrc(registers_.de.Low()); };
	// RRC H
	cb_instructions_[0x0C] = [this]() { registers_.hl.High() = Rrc(registers_.hl.High()); };
	// RRC L
	cb_instructions_[0x0D] = [this]() { registers_.hl.Low() = Rrc(registers_.hl.Low()); };
	// RRC (HL)
	cb_instructions_[0x0E] = [this]() { WriteByte(registers_.hl, Rrc(ReadByte(registers_.hl))); };
	// RRC A
	cb_instructions_[0x0F] = [this]() { registers_.af.High() = Rrc(registers_.af.High()); };
#pragma endregion

#pragma region CB instructions 0x10 - 0x1F
	// RL B
	cb_instructions_[0x10] = [this]() { registers_.bc.High() = Rl(registers_.bc.High()); };
	// RL C
	cb_instructions_[0x11] = [this]() { registers_.bc.Low() = Rl(registers_.bc.Low()); };
	// RL D
	cb_instructions_[0x12] = [this]() { registers_.de.High() = Rl(registers_.de.High()); };
	// RL E
	cb_instructions_[0x13] = [this]() { registers_.de.Low() = Rl(registers_.de.Low()); };
	// RL H
	cb_instructions_[0x14] = [this]() { registers_.hl.High() = Rl(registers_.hl.High()); };
	// RL L
	cb_instructions_[0x15] = [this]() { registers_.hl.Low() = Rl(registers_.hl.Low()); };
	// RL (HL)
	cb_instructions_[0x16] = [this]() { WriteByte(registers_.hl, Rl(ReadByte(registers_.hl))); };
	// RL A
	cb_instructions_[0x17] = [this]() { registers_.af.High() = Rl(registers_.af.High()); };
	// RR B
	cb_instructions_[0x18] = [this]() { registers_.bc.High() = Rr(registers_.bc.High()); };
	// RR C
	cb_instructions_[0x19] = [this]() { registers_.bc.Low() = Rr(registers_.bc.Low()); };
	// RR D
	cb_instructions_[0x1A] = [this]() { registers_.de.High() = Rr(registers_.de.High()); };
	// RR E
	cb_instructions_[0x1B] = [this]() { registers_.de.Low() = Rr(registers_.de.Low()); };
	// RR H
	cb_instructions_[0x1C] = [this]() { registers_.hl.High() = Rr(registers_.hl.High()); };
	// RR L
	cb_instructions_[0x1D] = [this]() { registers_.hl.Low() = Rr(registers_.hl.Low()); };
	// RR (HL)
	cb_instructions_[0x1E] = [this]() { WriteByte(registers_.hl, Rr(ReadByte(registers_.hl))); };
	// RR A
	cb_instructions_[0x1F] = [this]() { registers_.af.High() = Rr(registers_.af.High()); };
#pragma endregion

#pragma region CB instructions 0x20 - 0x2F
	// SLA B
	cb_instructions_[0x20] = [this]() { registers_.bc.High() = Sla(registers_.bc.High()); };
	// SLA C
	cb_instructions_[0x21] = [this]() { registers_.bc.Low() = Sla(registers_.bc.Low()); };
	// SLA D
	cb_instructions_[0x22] = [this]() { registers_.de.High() = Sla(registers_.de.High()); };
	// SLA E
	cb_instructions_[0x23] = [this]() { registers_.de.Low() = Sla(registers_.de.Low()); };
	// SLA H
	cb_instructions_[0x24] = [this]() { registers_.hl.High() = Sla(registers_.hl.High());};
	// SLA L
	cb_instructions_[0x25] = [this]() { registers_.hl.Low() = Sla(registers_.hl.Low()); };
	// SLA (HL)
	cb_instructions_[0x26] = [this]() { WriteByte(registers_.hl, Sla(ReadByte(registers_.hl))); };
	// SLA A
	cb_instructions_[0x27] = [this]() { registers_.af.High() = Sla(registers_.af.High()); };
	// SRA B
	cb_instructions_[0x28] = [this]() { registers_.bc.High() = Sra(registers_.bc.High()); };
	// SRA C
	cb_instructions_[0x29] = [this]() { registers_.bc.Low() = Sra(registers_.bc.Low()); };
	// SRA D
	cb_instructions_[0x2A] = [this]() { registers_.de.High() = Sra(registers_.de.High()); };
	// SRA E
	cb_instructions_[0x2B] = [this]() { registers_.de.Low() = Sra(registers_.de.Low()); };
	// SRA H
	cb_instructions_[0x2C] = [this]() { registers_.hl.High() = Sra(registers_.hl.High()); };
	// SRA L
	cb_instructions_[0x2D] = [this]() { registers_.hl.Low() = Sra(registers_.hl.Low()); };
	// SRA (HL)
	cb_instructions_[0x2E] = [this]() { WriteByte(registers_.hl, Sra(ReadByte(registers_.hl))); };
	// SRA A
	cb_instructions_[0x2F] = [this]() { registers_.af.High() = Sra(registers_.af.High()); };
#pragma endregion

#pragma region CB instructions 0x30 - 0x3F
	// SWAP B
	cb_instructions_[0x30] = [this]() { registers_.bc.High() = Swap(registers_.bc.High()); };
	// SWAP C
	cb_instructions_[0x31] = [this]() { registers_.bc.Low() = Swap(registers_.bc.Low()); };
	// SWAP D
	cb_instructions_[0x32] = [this]() { registers_.de.High() = Swap(registers_.de.High()); };
	// SWAP E
	cb_instructions_[0x33] = [this]() { registers_.de.Low() = Swap(registers_.de.Low()); };
	// SWAP H
	cb_instructions_[0x34] = [this]() { registers_.hl.High() = Swap(registers_.hl.High()); };
	// SWAP L
	cb_instructions_[0x35] = [this]() { registers_.hl.Low() = Swap(registers_.hl.Low()); };
	// SWAP (HL)
	cb_instructions_[0x36] = [this]() { WriteByte(registers_.hl, Swap(ReadByte(registers_.hl))); };
	// SWAP A
	cb_instructions_[0x37] = [this]() { registers_.af.High() = Swap(registers_.af.High()); };
	// SRL B
	cb_instructions_[0x38] = [this]() { registers_.bc.High() = Srl(registers_.bc.High()); };
	// SRL C
	cb_instructions_[0x39] = [this]() { registers_.bc.Low() = Srl(registers_.bc.Low()); };
	// SRL D
	cb_instructions_[0x3A] = [this]() { registers_.de.High() = Srl(registers_.de.High()); };
	// SRL E
	cb_instructions_[0x3B] = [this]() { registers_.de.Low() = Srl(registers_.de.Low()); };
	// SRL H
	cb_instructions_[0x3C] = [this]() { registers_.hl.High() = Srl(registers_.hl.High()); };
	// SRL L
	cb_instructions_[0x3D] = [this]() { registers_.hl.Low() = Srl(registers_.hl.Low()); };
	// SRL (HL)
	cb_instructions_[0x3E] = [this]() { WriteByte(registers_.hl, Srl(ReadByte(registers_.hl))); };
	// SRL A
	cb_instructions_[0x3F] = [this]() { registers_.af.High() = Srl(registers_.af.High()); };
#pragma endregion

#pragma region CB instructions 0x40 - 0x4F
	// BIT 0, B
	cb_instructions_[0x40] = [this]() { Test(registers_.bc.High(), 0x01); };
	// BIT 0, C
	cb_instructions_[0x41] = [this]() { Test(registers_.bc.Low(), 0x01); };
	// BIT 0, D
	cb_instructions_[0x42] = [this]() { Test(registers_.de.High(), 0x01); };
	// BIT 0, E
	cb_instructions_[0x43] = [this]() { Test(registers_.de.Low(), 0x01); };
	// BIT 0, H
	cb_instructions_[0x44] = [this]() { Test(registers_.hl.High(), 0x01); };
	// BIT 0, L
	cb_instructions_[0x45] = [this]() { Test(registers_.hl.Low(), 0x01); };
	// BIT 0, (HL)
	cb_instructions_[0x46] = [this]() { Test(ReadByte(registers_.hl), 0x01); };
	// BIT 0, A
	cb_instructions_[0x47] = [this]() { Test(registers_.af.High(), 0x01); };
	// BIT 1, B
	cb_instructions_[0x48] = [this]() { Test(registers_.bc.High(), 0x02); };
	// BIT 1, C
	cb_instructions_[0x49] = [this]() { Test(registers_.bc.Low(), 0x02); };
	// BIT 1, D
	cb_instructions_[0x4A] = [this]() { Test(registers_.de.High(), 0x02); };
	// BIT 1, E
	cb_instructions_[0x4B] = [this]() { Test(registers_.de.Low(), 0x02); };
	// BIT 1, H
	cb_instructions_[0x4C] = [this]() { Test(registers_.hl.High(), 0x02); };
	// BIT 1, L
	cb_instructions_[0x4D] = [this]() { Test(registers_.hl.Low(), 0x02); };
	// BIT 1, (HL)
	cb_instructions_[0x4E] = [this]() { Test(ReadByte(registers_.hl), 0x02); };
	// BIT 1, A
	cb_instructions_[0x4F] = [this]() { Test(registers_.af.High(), 0x02); };
#pragma endregion

#pragma region CB instructions 0x50 - 0x5F
	// BIT 2, B
	cb_instructions_[0x50] = [this]() { Test(registers_.bc.High(), 0x04); };
	// BIT 2, C
	cb_instructions_[0x51] = [this]() { Test(registers_.bc.Low(), 0x04); };
	// BIT 2, D
	cb_instructions_[0x52] = [this]() { Test(registers_.de.High(), 0x04); };
	// BIT 2, E
	cb_instructions_[0x53] = [this]() { Test(registers_.de.Low(), 0x04); };
	// BIT 2, H
	cb_instructions_[0x54] = [this]() { Test(registers_.hl.High(), 0x04); };
	// BIT 2, L
	cb_instructions_[0x55] = [this]() { Test(registers_.hl.Low(), 0x04); };
	// BIT 2, (HL)
	cb_instructions_[0x56] = [this]() { Test(ReadByte(registers_.hl), 0x04); };
	// BIT 2, A
	cb_instructions_[0x57] = [this]() { Test(registers_.af.High(), 0x04); };
	// BIT 3, B
	cb_instructions_[0x58] = [this]() { Test(registers_.bc.High(), 0x08); };
	// BIT 3, C
	cb_instructions_[0x59] = [this]() { Test(registers_.bc.Low(), 0x08); };
	// BIT 3, D
	cb_instructions_[0x5A] = [this]() { Test(registers_.de.High(), 0x08); };
	// BIT 3, E
	cb_instructions_[0x5B] = [this]() { Test(registers_.de.Low(), 0x08); };
	// BIT 3, H
	cb_instructions_[0x5C] = [this]() { Test(registers_.hl.High(), 0x08); };
	// BIT 3, L
	cb_instructions_[0x5D] = [this]() { Test(registers_.hl.Low(), 0x08); };
	// BIT 3, (HL)
	cb_instructions_[0x5E] = [this]() { Test(ReadByte(registers_.hl), 0x08); };
	// BIT 3, A
	cb_instructions_[0x5F] = [this]() { Test(registers_.af.High(), 0x08); };
#pragma endregion

#pragma region CB instructions 0x60 - 0x6F
	// BIT 4, B
	cb_instructions_[0x60] = [this]() { Test(registers_.bc.High(), 0x10); };
	// BIT 4, C
	cb_instructions_[0x61] = [this]() { Test(registers_.bc.Low(), 0x10); };
	// BIT 4, D
	cb_instructions_[0x62] = [this]() { Test(registers_.de.High(), 0x10); };
	// BIT 4, E
	cb_instructions_[0x63] = [this]() { Test(registers_.de.Low(), 0x10); };
	// BIT 4, H
	cb_instructions_[0x64] = [this]() { Test(registers_.hl.High(), 0x10); };
	// BIT 4, L
	cb_instructions_[0x65] = [this]() { Test(registers_.hl.Low(), 0x10); };
	// BIT 4, (HL)
	cb_instructions_[0x66] = [this]() { Test(ReadByte(registers_.hl), 0x10); };
	// BIT 4, A
	cb_instructions_[0x67] = [this]() { Test(registers_.af.High(), 0x10); };
	// BIT 5, B
	cb_instructions_[0x68] = [this]() { Test(registers_.bc.High(), 0x20); };
	// BIT 5, C
	cb_instructions_[0x69] = [this]() { Test(registers_.bc.Low(), 0x20); };
	// BIT 5, D
	cb_instructions_[0x6A] = [this]() { Test(registers_.de.High(), 0x20); };
	// BIT 5, E
	cb_instructions_[0x6B] = [this]() { Test(registers_.de.Low(), 0x20); };
	// BIT 5, H
	cb_instructions_[0x6C] = [this]() { Test(registers_.hl.High(), 0x20); };
	// BIT 5, L
	cb_instructions_[0x6D] = [this]() { Test(registers_.hl.Low(), 0x20); };
	// BIT 5, (HL)
	cb_instructions_[0x6E] = [this]() { Test(ReadByte(registers_.hl), 0x20); };
	// BIT 5, A
	cb_instructions_[0x6F] = [this]() { Test(registers_.af.High(), 0x20); };
#pragma endregion

#pragma region CB instructions 0x70 - 0x7F
	// BIT 6, B
	cb_instructions_[0x70] = [this]() { Test(registers_.bc.High(), 0x40); };
	// BIT 6, C
	cb_instructions_[0x71] = [this]() { Test(registers_.bc.Low(), 0x40); }; 
	// BIT 6, D
	cb_instructions_[0x72] = [this]() { Test(registers_.de.High(), 0x40); }; 
	// BIT 6, E
	cb_instructions_[0x73] = [this]() { Test(registers_.de.Low(), 0x40); };
	// BIT 6, H
	cb_instructions_[0x74] = [this]() { Test(registers_.hl.High(), 0x40); };
	// BIT 6, L
	cb_instructions_[0x75] = [this]() { Test(registers_.hl.Low(), 0x40); };
	// BIT 6, (HL)
	cb_instructions_[0x76] = [this]() { Test(ReadByte(registers_.hl), 0x40); };
	// BIT 6, A
	cb_instructions_[0x77] = [this]() { Test(registers_.af.High(), 0x40);};
	// BIT 7, B
	cb_instructions_[0x78] = [this]() { Test(registers_.bc.High(), 0x80); };
	// BIT 7, C
	cb_instructions_[0x79] = [this]() { Test(registers_.bc.Low(), 0x80); };
	// BIT 7, D
	cb_instructions_[0x7A] = [this]() { Test(registers_.de.High(), 0x80); };
	// BIT 7, E
	cb_instructions_[0x7B] = [this]() { Test(registers_.de.Low(), 0x80); };
	// BIT 7, H
	cb_instructions_[0x7C] = [this]() { Test(registers_.hl.High(), 0x80); };
	// BIT 7, L
	cb_instructions_[0x7D] = [this]() { Test(registers_.hl.Low(), 0x80); };
	// BIT 7, (HL)
	cb_instructions_[0x7E] = [this]() { Test(ReadByte(registers_.hl), 0x80); };
	// BIT 7, A
	cb_instructions_[0x7F] = [this]() { Test(registers_.af.High(), 0x80); };
#pragma endregion

#pragma region CB instructions 0x80 - 0x8F
	// RES 0, B
	cb_instructions_[0x80] = [this]() { registers_.bc.High() &= 0xFE; };
	// RES 0, C
	cb_instructions_[0x81] = [this]() { registers_.bc.Low() &= 0xFE; };
	// RES 0, D
	cb_instructions_[0x82] = [this]() { registers_.de.High() &= 0xFE; };
	// RES 0, E
	cb_instructions_[0x83] = [this]() { registers_.de.Low() &= 0xFE; };
	// RES 0, H
	cb_instructions_[0x84] = [this]() { registers_.hl.High() &= 0xFE; };
	// RES 0, L
	cb_instructions_[0x85] = [this]() { registers_.hl.Low() &= 0xFE; };
	// RES 0, (HL)
	cb_instructions_[0x86] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFE ); };
	// RES 0, A
	cb_instructions_[0x87] = [this]() { registers_.af.High() &= 0xFE; };
	// RES 1, B
	cb_instructions_[0x88] = [this]() { registers_.bc.High() &= 0xFD; };
	// RES 1, C
	cb_instructions_[0x89] = [this]() { registers_.bc.Low() &= 0xFD; };
	// RES 1, D
	cb_instructions_[0x8A] = [this]() { registers_.de.High() &= 0xFD; };
	// RES 1, E
	cb_instructions_[0x8B] = [this]() { registers_.de.Low() &= 0xFD; };
	// RES 1, H
	cb_instructions_[0x8C] = [this]() { registers_.hl.High() &= 0xFD; };
	// RES 1, L
	cb_instructions_[0x8D] = [this]() { registers_.hl.Low() &= 0xFD; };
	// RES 1, (HL)
	cb_instructions_[0x8E] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFD); };
	// RES 1, A
	cb_instructions_[0x8F] = [this]() { registers_.af.High() &= 0xFD; };
#pragma endregion

#pragma region CB instructions 0x90 - 0x9F
	// RES 2, B
	cb_instructions_[0x90] = [this]() { registers_.bc.High() &= 0xFB; };
	// RES 2, C
	cb_instructions_[0x91] = [this]() { registers_.bc.Low() &= 0xFB; };
	// RES 2, D
	cb_instructions_[0x92] = [this]() { registers_.de.High() &= 0xFB; };
	// RES 2, E
	cb_instructions_[0x93] = [this]() { registers_.de.Low() &= 0xFB; };
	// RES 2, H
	cb_instructions_[0x94] = [this]() { registers_.hl.High() &= 0xFB; };
	// RES 2, L
	cb_instructions_[0x95] = [this]() { registers_.hl.Low() &= 0xFB; };
	// RES 2, (HL)
	cb_instructions_[0x96] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFB); };
	// RES 2, A
	cb_instructions_[0x97] = [this]() { registers_.af.High() &= 0xFB; };
	// RES 3, B
	cb_instructions_[0x98] = [this]() { registers_.bc.High() &= 0xF7; };
	// RES 3, C
	cb_instructions_[0x99] = [this]() { registers_.bc.Low() &= 0xF7; };
	// RES 3, D
	cb_instructions_[0x9A] = [this]() { registers_.de.High() &= 0xF7; };
	// RES 3, E
	cb_instructions_[0x9B] = [this]() { registers_.de.Low() &= 0xF7; };
	// RES 3, H
	cb_instructions_[0x9C] = [this]() { registers_.hl.High() &= 0xF7; };
	// RES 3, L
	cb_instructions_[0x9D] = [this]() { registers_.hl.Low() &= 0xF7; };
	// RES 3, (HL)
	cb_instructions_[0x9E] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xF7); };
	// RES 3, A
	cb_instructions_[0x9F] = [this]() { registers_.af.High() &= 0xF7; };
#pragma endregion

#pragma region CB instructions 0xA0 - 0xAF
	// RES 4, B
	cb_instructions_[0xA0] = [this]() { registers_.bc.High() &= 0xEF; };
	// RES 4, C
	cb_instructions_[0xA1] = [this]() { registers_.bc.Low() &= 0xEF; };
	// RES 4, D
	cb_instructions_[0xA2] = [this]() { registers_.de.High() &= 0xEF; };
	// RES 4, E
	cb_instructions_[0xA3] = [this]() { registers_.de.Low() &= 0xEF; };
	// RES 4, H
	cb_instructions_[0xA4] = [this]() { registers_.hl.High() &= 0xEF; };
	// RES 4, L
	cb_instructions_[0xA5] = [this]() { registers_.hl.Low() &= 0xEF; };
	// RES 4, (HL)
	cb_instructions_[0xA6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xEF); };
	// RES 4, A
	cb_instructions_[0xA7] = [this]() { registers_.af.High() &= 0xEF; };
	// RES 5, B
	cb_instructions_[0xA8] = [this]() { registers_.bc.High() &= 0xDF; };
	// RES 5, C
	cb_instructions_[0xA9] = [this]() { registers_.bc.Low() &= 0xDF; };
	// RES 5, D
	cb_instructions_[0xAA] = [this]() { registers_.de.High() &= 0xDF; };
	// RES 5, E
	cb_instructions_[0xAB] = [this]() { registers_.de.Low() &= 0xDF; };
	// RES 5, H
	cb_instructions_[0xAC] = [this]() { registers_.hl.High() &= 0xDF; };
	// RES 5, L
	cb_instructions_[0xAD] = [this]() { registers_.hl.Low() &= 0xDF; };
	// RES 5, (HL)
	cb_instructions_[0xAE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xDF); };
	// RES 5, A
	cb_instructions_[0xAF] = [this]() { registers_.af.High() &= 0xDF; };
#pragma endregion

#pragma region CB instructions 0xB0 - 0xBF
	// RES 6, B
	cb_instructions_[0xB0] = [this]() { registers_.bc.High() &= 0xBF; };
	// RES 6, C
	cb_instructions_[0xB1] = [this]() { registers_.bc.Low() &= 0xBF; };
	// RES 6, D
	cb_instructions_[0xB2] = [this]() { registers_.de.High() &= 0xBF; };
	// RES 6, E
	cb_instructions_[0xB3] = [this]() { registers_.de.Low() &= 0xBF; };
	// RES 6, H
	cb_instructions_[0xB4] = [this]() { registers_.hl.High() &= 0xBF; };
	// RES 6, L
	cb_instructions_[0xB5] = [this]() { registers_.hl.Low() &= 0xBF; };
	// RES 6, (HL)
	cb_instructions_[0xB6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xBF); };
	// RES 6, A
	cb_instructions_[0xB7] = [this]() { registers_.af.High() &= 0xBF; };
	// RES 7, B
	cb_instructions_[0xB8] = [this]() { registers_.bc.High() &= 0x7F; };
	// RES 7, C
	cb_instructions_[0xB9] = [this]() { registers_.bc.Low() &= 0x7F; };
	// RES 7, D
	cb_instructions_[0xBA] = [this]() { registers_.de.High() &= 0x7F; };
	// RES 7, E
	cb_instructions_[0xBB] = [this]() { registers_.de.Low() &= 0x7F; };
	// RES 7, H
	cb_instructions_[0xBC] = [this]() { registers_.hl.High() &= 0x7F; };
	// RES 7, L
	cb_instructions_[0xBD] = [this]() { registers_.hl.Low() &= 0x7F; };
	// RES 7, (HL)
	cb_instructions_[0xBE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) & 0x7F); };
	// RES 7, A
	cb_instructions_[0xBF] = [this]() { registers_.af.High() &= 0x7F; };
#pragma endregion

#pragma region CB instructions 0xC0 - 0xCF
	// SET 0, B
	cb_instructions_[0xC0] = [this]() { registers_.bc.High() |= 0x01; };
	// SET 0, C
	cb_instructions_[0xC1] = [this]() { registers_.bc.Low() |= 0x01; };
	// SET 0, D
	cb_instructions_[0xC2] = [this]() { registers_.de.High() |= 0x01; };
	// SET 0, E
	cb_instructions_[0xC3] = [this]() { registers_.de.Low() |= 0x01; };
	// SET 0, H
	cb_instructions_[0xC4] = [this]() { registers_.hl.High() |= 0x01; };
	// SET 0, L
	cb_instructions_[0xC5] = [this]() { registers_.hl.Low() |= 0x01; };
	// SET 0, (HL)
	cb_instructions_[0xC6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x01); };
	// SET 0, A
	cb_instructions_[0xC7] = [this]() { registers_.af.High() |= 0x01; };
	// SET 1, B
	cb_instructions_[0xC8] = [this]() { registers_.bc.High() |= 0x02; };
	// SET 1, C
	cb_instructions_[0xC9] = [this]() { registers_.bc.Low() |= 0x02; };
	// SET 1, D
	cb_instructions_[0xCA] = [this]() { registers_.de.High() |= 0x02; };
	// SET 1, E
	cb_instructions_[0xCB] = [this]() { registers_.de.Low() |= 0x02; };
	// SET 1, H
	cb_instructions_[0xCC] = [this]() { registers_.hl.High() |= 0x02; };
	// SET 1, L
	cb_instructions_[0xCD] = [this]() { registers_.hl.Low() |= 0x02; };
	// SET 1, (HL)
	cb_instructions_[0xCE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x02); };
	// SET 1, A
	cb_instructions_[0xCF] = [this]() { registers_.af.High() |= 0x02; };
#pragma endregion

#pragma region CB instructions 0xD0 - 0xDF
	// SET 2, B
	cb_instructions_[0xD0] = [this]() { registers_.bc.High() |= 0x04; };
	// SET 2, C
	cb_instructions_[0xD1] = [this]() { registers_.bc.Low() |= 0x04; };
	// SET 2, D
	cb_instructions_[0xD2] = [this]() { registers_.de.High() |= 0x04; };
	// SET 2, E
	cb_instructions_[0xD3] = [this]() { registers_.de.Low() |= 0x04; };
	// SET 2, H
	cb_instructions_[0xD4] = [this]() { registers_.hl.High() |= 0x04; };
	// SET 2, L
	cb_instructions_[0xD5] = [this]() { registers_.hl.Low() |= 0x04; };
	// SET 2, (HL)
	cb_instructions_[0xD6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x04); };
	// SET 2, A
	cb_instructions_[0xD7] = [this]() { registers_.af.High() |= 0x04; };
	// SET 3, B
	cb_instructions_[0xD8] = [this]() { registers_.bc.High() |= 0x08; };
	// SET 3, C
	cb_instructions_[0xD9] = [this]() { registers_.bc.Low() |= 0x08; };
	// SET 3, D
	cb_instructions_[0xDA] = [this]() { registers_.de.High() |= 0x08; };
	// SET 3, E
	cb_instructions_[0xDB] = [this]() { registers_.de.Low() |= 0x08; };
	// SET 3, H
	cb_instructions_[0xDC] = [this]() { registers_.hl.High() |= 0x08; };
	// SET 3, L
	cb_instructions_[0xDD] = [this]() { registers_.hl.Low() |= 0x08; };
	// SET 3, (HL)
	cb_instructions_[0xDE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x08); };
	// SET 3, A
	cb_instructions_[0xDF] = [this]() { registers_.af.High() |= 0x08; };
#pragma endregion

#pragma region CB instructions 0xE0 - 0xEF
	// SET 4, B
	cb_instructions_[0xE0] = [this]() { registers_.bc.High() |= 0x10; };
	// SET 4, C
	cb_instructions_[0xE1] = [this]() { registers_.bc.Low() |= 0x10; };
	// SET 4, D
	cb_instructions_[0xE2] = [this]() { registers_.de.High() |= 0x10; };
	// SET 4, E
	cb_instructions_[0xE3] = [this]() { registers_.de.Low() |= 0x10; };
	// SET 4, H
	cb_instructions_[0xE4] = [this]() { registers_.hl.High() |= 0x10; };
	// SET 4, L
	cb_instructions_[0xE5] = [this]() { registers_.hl.Low() |= 0x10; };
	// SET 4, (HL)
	cb_instructions_[0xE6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x10); };
	// SET 4, A
	cb_instructions_[0xE7] = [this]() { registers_.af.High() |= 0x10; };
	// SET 5, B
	cb_instructions_[0xE8] = [this]() { registers_.bc.High() |= 0x20; };
	// SET 5, C
	cb_instructions_[0xE9] = [this]() { registers_.bc.Low() |= 0x20; };
	// SET 5, D
	cb_instructions_[0xEA] = [this]() { registers_.de.High() |= 0x20; };
	// SET 5, E
	cb_instructions_[0xEB] = [this]() { registers_.de.Low() |= 0x20; };
	// SET 5, H
	cb_instructions_[0xEC] = [this]() { registers_.hl.High() |= 0x20; };
	// SET 5, L
	cb_instructions_[0xED] = [this]() { registers_.hl.Low() |= 0x20; };
	// SET 5, (HL)
	cb_instructions_[0xEE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x20); };
	// SET 5, A
	cb_instructions_[0xEF] = [this]() { registers_.af.High() |= 0x20; };
#pragma endregion

#pragma region CB instructions 0xF0 - 0xFF
	// SET 6, B
	cb_instructions_[0xF0] = [this]() { registers_.bc.High() |= 0x40; };
	// SET 6, C
	cb_instructions_[0xF1] = [this]() { registers_.bc.Low() |= 0x40; };
	// SET 6, D
	cb_instructions_[0xF2] = [this]() { registers_.de.High() |= 0x40; };
	// SET 6, E
	cb_instructions_[0xF3] = [this]() { registers_.de.Low() |= 0x40; };
	// SET 6, H
	cb_instructions_[0xF4] = [this]() { registers_.hl.High() |= 0x40; };
	// SET 6, L
	cb_instructions_[0xF5] = [this]() { registers_.hl.Low() |= 0x40; };
	// SET 6, (HL)
	cb_instructions_[0xF6] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x40); };
	// SET 6, A
	cb_instructions_[0xF7] = [this]() { registers_.af.High() |= 0x40; };
	// SET 7, B
	cb_instructions_[0xF8] = [this]() { registers_.bc.High() |= 0x80; };
	// SET 7, C
	cb_instructions_[0xF9] = [this]() { registers_.bc.Low() |= 0x80; };
	// SET 7, D
	cb_instructions_[0xFA] = [this]() { registers_.de.High() |= 0x80; };
	// SET 7, E
	cb_instructions_[0xFB] = [this]() { registers_.de.Low() |= 0x80; };
	// SET 7, H
	cb_instructions_[0xFC] = [this]() { registers_.hl.High() |= 0x80; };
	// SET 7, L
	cb_instructions_[0xFD] = [this]() { registers_.hl.Low() |= 0x80; };
	// SET 7, (HL)
	cb_instructions_[0xFE] = [this]() { WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x80); };
	// SET 7, A
	cb_instructions_[0xFF] = [this]() { registers_.af.High() |= 0x80; };
#pragma endregion
}
