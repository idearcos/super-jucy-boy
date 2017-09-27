#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"
#include "InstructionMnemonics.h"

void CPU::ExecuteCbInstruction(OpCode opcode)
{
	switch (opcode)
	{
#pragma region CB instructions 0x00 - 0x0F
		// RLC B
		case 0x00: registers_.bc.High() = Rlc(registers_.bc.High()); break;
		// RLC C
		case 0x01: registers_.bc.Low() = Rlc(registers_.bc.Low()); break;
		// RLC D
		case 0x02: registers_.de.High() = Rlc(registers_.de.High()); break;
		// RLC E
		case 0x03: registers_.de.Low() = Rlc(registers_.de.Low()); break;
		// RLC H
		case 0x04: registers_.hl.High() = Rlc(registers_.hl.High()); break;
		// RLC L
		case 0x05: registers_.hl.Low() = Rlc(registers_.hl.Low()); break;
		// RLC (HL)
		case 0x06: WriteByte(registers_.hl, Rlc(ReadByte(registers_.hl))); break;
		// RLC A
		case 0x07: registers_.af.High() = Rlc(registers_.af.High()); break;
		// RRC B
		case 0x08: registers_.bc.High() = Rrc(registers_.bc.High()); break;
		// RRC C
		case 0x09: registers_.bc.Low() = Rrc(registers_.bc.Low()); break;
		// RRC D
		case 0x0A: registers_.de.High() = Rrc(registers_.de.High()); break;
		// RRC E
		case 0x0B: registers_.de.Low() = Rrc(registers_.de.Low()); break;
		// RRC H
		case 0x0C: registers_.hl.High() = Rrc(registers_.hl.High()); break;
		// RRC L
		case 0x0D: registers_.hl.Low() = Rrc(registers_.hl.Low()); break;
		// RRC (HL)
		case 0x0E: WriteByte(registers_.hl, Rrc(ReadByte(registers_.hl))); break;
		// RRC A
		case 0x0F: registers_.af.High() = Rrc(registers_.af.High()); break;
#pragma endregion

#pragma region CB instructions 0x10 - 0x1F
		// RL B
		case 0x10: registers_.bc.High() = Rl(registers_.bc.High()); break;
		// RL C
		case 0x11: registers_.bc.Low() = Rl(registers_.bc.Low()); break;
		// RL D
		case 0x12: registers_.de.High() = Rl(registers_.de.High()); break;
		// RL E
		case 0x13: registers_.de.Low() = Rl(registers_.de.Low()); break;
		// RL H
		case 0x14: registers_.hl.High() = Rl(registers_.hl.High()); break;
		// RL L
		case 0x15: registers_.hl.Low() = Rl(registers_.hl.Low()); break;
		// RL (HL)
		case 0x16: WriteByte(registers_.hl, Rl(ReadByte(registers_.hl))); break;
		// RL A
		case 0x17: registers_.af.High() = Rl(registers_.af.High()); break;
		// RR B
		case 0x18: registers_.bc.High() = Rr(registers_.bc.High()); break;
		// RR C
		case 0x19: registers_.bc.Low() = Rr(registers_.bc.Low()); break;
		// RR D
		case 0x1A: registers_.de.High() = Rr(registers_.de.High()); break;
		// RR E
		case 0x1B: registers_.de.Low() = Rr(registers_.de.Low()); break;
		// RR H
		case 0x1C: registers_.hl.High() = Rr(registers_.hl.High()); break;
		// RR L
		case 0x1D: registers_.hl.Low() = Rr(registers_.hl.Low()); break;
		// RR (HL)
		case 0x1E: WriteByte(registers_.hl, Rr(ReadByte(registers_.hl))); break;
		// RR A
		case 0x1F: registers_.af.High() = Rr(registers_.af.High()); break;
#pragma endregion

#pragma region CB instructions 0x20 - 0x2F
		// SLA B
		case 0x20: registers_.bc.High() = Sla(registers_.bc.High()); break;
		// SLA C
		case 0x21: registers_.bc.Low() = Sla(registers_.bc.Low()); break;
		// SLA D
		case 0x22: registers_.de.High() = Sla(registers_.de.High()); break;
		// SLA E
		case 0x23: registers_.de.Low() = Sla(registers_.de.Low()); break;
		// SLA H
		case 0x24: registers_.hl.High() = Sla(registers_.hl.High()); break;
		// SLA L
		case 0x25: registers_.hl.Low() = Sla(registers_.hl.Low()); break;
		// SLA (HL)
		case 0x26: WriteByte(registers_.hl, Sla(ReadByte(registers_.hl))); break;
		// SLA A
		case 0x27: registers_.af.High() = Sla(registers_.af.High()); break;
		// SRA B
		case 0x28: registers_.bc.High() = Sra(registers_.bc.High()); break;
		// SRA C
		case 0x29: registers_.bc.Low() = Sra(registers_.bc.Low()); break;
		// SRA D
		case 0x2A: registers_.de.High() = Sra(registers_.de.High()); break;
		// SRA E
		case 0x2B: registers_.de.Low() = Sra(registers_.de.Low()); break;
		// SRA H
		case 0x2C: registers_.hl.High() = Sra(registers_.hl.High()); break;
		// SRA L
		case 0x2D: registers_.hl.Low() = Sra(registers_.hl.Low()); break;
		// SRA (HL)
		case 0x2E: WriteByte(registers_.hl, Sra(ReadByte(registers_.hl))); break;
		// SRA A
		case 0x2F: registers_.af.High() = Sra(registers_.af.High()); break;
#pragma endregion

#pragma region CB instructions 0x30 - 0x3F
		// SWAP B
		case 0x30: registers_.bc.High() = Swap(registers_.bc.High()); break;
		// SWAP C
		case 0x31: registers_.bc.Low() = Swap(registers_.bc.Low()); break;
		// SWAP D
		case 0x32: registers_.de.High() = Swap(registers_.de.High()); break;
		// SWAP E
		case 0x33: registers_.de.Low() = Swap(registers_.de.Low()); break;
		// SWAP H
		case 0x34: registers_.hl.High() = Swap(registers_.hl.High()); break;
		// SWAP L
		case 0x35: registers_.hl.Low() = Swap(registers_.hl.Low()); break;
		// SWAP (HL)
		case 0x36: WriteByte(registers_.hl, Swap(ReadByte(registers_.hl))); break;
		// SWAP A
		case 0x37: registers_.af.High() = Swap(registers_.af.High()); break;
		// SRL B
		case 0x38: registers_.bc.High() = Srl(registers_.bc.High()); break;
		// SRL C
		case 0x39: registers_.bc.Low() = Srl(registers_.bc.Low()); break;
		// SRL D
		case 0x3A: registers_.de.High() = Srl(registers_.de.High()); break;
		// SRL E
		case 0x3B: registers_.de.Low() = Srl(registers_.de.Low()); break;
		// SRL H
		case 0x3C: registers_.hl.High() = Srl(registers_.hl.High()); break;
		// SRL L
		case 0x3D: registers_.hl.Low() = Srl(registers_.hl.Low()); break;
		// SRL (HL)
		case 0x3E: WriteByte(registers_.hl, Srl(ReadByte(registers_.hl))); break;
		// SRL A
		case 0x3F: registers_.af.High() = Srl(registers_.af.High()); break;
#pragma endregion

#pragma region CB instructions 0x40 - 0x4F
		// BIT 0, B
		case 0x40: Test(registers_.bc.High(), 0x01); break;
		// BIT 0, C
		case 0x41: Test(registers_.bc.Low(), 0x01); break;
		// BIT 0, D
		case 0x42: Test(registers_.de.High(), 0x01); break;
		// BIT 0, E
		case 0x43: Test(registers_.de.Low(), 0x01); break;
		// BIT 0, H
		case 0x44: Test(registers_.hl.High(), 0x01); break;
		// BIT 0, L
		case 0x45: Test(registers_.hl.Low(), 0x01); break;
		// BIT 0, (HL)
		case 0x46: Test(ReadByte(registers_.hl), 0x01); break;
		// BIT 0, A
		case 0x47: Test(registers_.af.High(), 0x01); break;
		// BIT 1, B
		case 0x48: Test(registers_.bc.High(), 0x02); break;
		// BIT 1, C
		case 0x49: Test(registers_.bc.Low(), 0x02); break;
		// BIT 1, D
		case 0x4A: Test(registers_.de.High(), 0x02); break;
		// BIT 1, E
		case 0x4B: Test(registers_.de.Low(), 0x02); break;
		// BIT 1, H
		case 0x4C: Test(registers_.hl.High(), 0x02); break;
		// BIT 1, L
		case 0x4D: Test(registers_.hl.Low(), 0x02); break;
		// BIT 1, (HL)
		case 0x4E: Test(ReadByte(registers_.hl), 0x02); break;
		// BIT 1, A
		case 0x4F: Test(registers_.af.High(), 0x02); break;
#pragma endregion

#pragma region CB instructions 0x50 - 0x5F
		// BIT 2, B
		case 0x50: Test(registers_.bc.High(), 0x04); break;
		// BIT 2, C
		case 0x51: Test(registers_.bc.Low(), 0x04); break;
		// BIT 2, D
		case 0x52: Test(registers_.de.High(), 0x04); break;
		// BIT 2, E
		case 0x53: Test(registers_.de.Low(), 0x04); break;
		// BIT 2, H
		case 0x54: Test(registers_.hl.High(), 0x04); break;
		// BIT 2, L
		case 0x55: Test(registers_.hl.Low(), 0x04); break;
		// BIT 2, (HL)
		case 0x56: Test(ReadByte(registers_.hl), 0x04); break;
		// BIT 2, A
		case 0x57: Test(registers_.af.High(), 0x04); break;
		// BIT 3, B
		case 0x58: Test(registers_.bc.High(), 0x08); break;
		// BIT 3, C
		case 0x59: Test(registers_.bc.Low(), 0x08); break;
		// BIT 3, D
		case 0x5A: Test(registers_.de.High(), 0x08); break;
		// BIT 3, E
		case 0x5B: Test(registers_.de.Low(), 0x08); break;
		// BIT 3, H
		case 0x5C: Test(registers_.hl.High(), 0x08); break;
		// BIT 3, L
		case 0x5D: Test(registers_.hl.Low(), 0x08); break;
		// BIT 3, (HL)
		case 0x5E: Test(ReadByte(registers_.hl), 0x08); break;
		// BIT 3, A
		case 0x5F: Test(registers_.af.High(), 0x08); break;
#pragma endregion

#pragma region CB instructions 0x60 - 0x6F
		// BIT 4, B
		case 0x60: Test(registers_.bc.High(), 0x10); break;
		// BIT 4, C
		case 0x61: Test(registers_.bc.Low(), 0x10); break;
		// BIT 4, D
		case 0x62: Test(registers_.de.High(), 0x10); break;
		// BIT 4, E
		case 0x63: Test(registers_.de.Low(), 0x10); break;
		// BIT 4, H
		case 0x64: Test(registers_.hl.High(), 0x10); break;
		// BIT 4, L
		case 0x65: Test(registers_.hl.Low(), 0x10); break;
		// BIT 4, (HL)
		case 0x66: Test(ReadByte(registers_.hl), 0x10); break;
		// BIT 4, A
		case 0x67: Test(registers_.af.High(), 0x10); break;
		// BIT 5, B
		case 0x68: Test(registers_.bc.High(), 0x20); break;
		// BIT 5, C
		case 0x69: Test(registers_.bc.Low(), 0x20); break;
		// BIT 5, D
		case 0x6A: Test(registers_.de.High(), 0x20); break;
		// BIT 5, E
		case 0x6B: Test(registers_.de.Low(), 0x20); break;
		// BIT 5, H
		case 0x6C: Test(registers_.hl.High(), 0x20); break;
		// BIT 5, L
		case 0x6D: Test(registers_.hl.Low(), 0x20); break;
		// BIT 5, (HL)
		case 0x6E: Test(ReadByte(registers_.hl), 0x20); break;
		// BIT 5, A
		case 0x6F: Test(registers_.af.High(), 0x20); break;
#pragma endregion

#pragma region CB instructions 0x70 - 0x7F
		// BIT 6, B
		case 0x70: Test(registers_.bc.High(), 0x40); break;
		// BIT 6, C
		case 0x71: Test(registers_.bc.Low(), 0x40); break;
		// BIT 6, D
		case 0x72: Test(registers_.de.High(), 0x40); break;
		// BIT 6, E
		case 0x73: Test(registers_.de.Low(), 0x40); break;
		// BIT 6, H
		case 0x74: Test(registers_.hl.High(), 0x40); break;
		// BIT 6, L
		case 0x75: Test(registers_.hl.Low(), 0x40); break;
		// BIT 6, (HL)
		case 0x76: Test(ReadByte(registers_.hl), 0x40); break;
		// BIT 6, A
		case 0x77: Test(registers_.af.High(), 0x40); break;
		// BIT 7, B
		case 0x78: Test(registers_.bc.High(), 0x80); break;
		// BIT 7, C
		case 0x79: Test(registers_.bc.Low(), 0x80); break;
		// BIT 7, D
		case 0x7A: Test(registers_.de.High(), 0x80); break;
		// BIT 7, E
		case 0x7B: Test(registers_.de.Low(), 0x80); break;
		// BIT 7, H
		case 0x7C: Test(registers_.hl.High(), 0x80); break;
		// BIT 7, L
		case 0x7D: Test(registers_.hl.Low(), 0x80); break;
		// BIT 7, (HL)
		case 0x7E: Test(ReadByte(registers_.hl), 0x80); break;
		// BIT 7, A
		case 0x7F: Test(registers_.af.High(), 0x80); break;
#pragma endregion

#pragma region CB instructions 0x80 - 0x8F
		// RES 0, B
		case 0x80: registers_.bc.High() &= 0xFE; break;
		// RES 0, C
		case 0x81: registers_.bc.Low() &= 0xFE; break;
		// RES 0, D
		case 0x82: registers_.de.High() &= 0xFE; break;
		// RES 0, E
		case 0x83: registers_.de.Low() &= 0xFE; break;
		// RES 0, H
		case 0x84: registers_.hl.High() &= 0xFE; break;
		// RES 0, L
		case 0x85: registers_.hl.Low() &= 0xFE; break;
		// RES 0, (HL)
		case 0x86: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFE); break;
		// RES 0, A
		case 0x87: registers_.af.High() &= 0xFE; break;
		// RES 1, B
		case 0x88: registers_.bc.High() &= 0xFD; break;
		// RES 1, C
		case 0x89: registers_.bc.Low() &= 0xFD; break;
		// RES 1, D
		case 0x8A: registers_.de.High() &= 0xFD; break;
		// RES 1, E
		case 0x8B: registers_.de.Low() &= 0xFD; break;
		// RES 1, H
		case 0x8C: registers_.hl.High() &= 0xFD; break;
		// RES 1, L
		case 0x8D: registers_.hl.Low() &= 0xFD; break;
		// RES 1, (HL)
		case 0x8E: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFD); break;
		// RES 1, A
		case 0x8F: registers_.af.High() &= 0xFD; break;
#pragma endregion

#pragma region CB instructions 0x90 - 0x9F
		// RES 2, B
		case 0x90: registers_.bc.High() &= 0xFB; break;
		// RES 2, C
		case 0x91: registers_.bc.Low() &= 0xFB; break;
		// RES 2, D
		case 0x92: registers_.de.High() &= 0xFB; break;
		// RES 2, E
		case 0x93: registers_.de.Low() &= 0xFB; break;
		// RES 2, H
		case 0x94: registers_.hl.High() &= 0xFB; break;
		// RES 2, L
		case 0x95: registers_.hl.Low() &= 0xFB; break;
		// RES 2, (HL)
		case 0x96: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xFB); break;
		// RES 2, A
		case 0x97: registers_.af.High() &= 0xFB; break;
		// RES 3, B
		case 0x98: registers_.bc.High() &= 0xF7; break;
		// RES 3, C
		case 0x99: registers_.bc.Low() &= 0xF7; break;
		// RES 3, D
		case 0x9A: registers_.de.High() &= 0xF7; break;
		// RES 3, E
		case 0x9B: registers_.de.Low() &= 0xF7; break;
		// RES 3, H
		case 0x9C: registers_.hl.High() &= 0xF7; break;
		// RES 3, L
		case 0x9D: registers_.hl.Low() &= 0xF7; break;
		// RES 3, (HL)
		case 0x9E: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xF7); break;
		// RES 3, A
		case 0x9F: registers_.af.High() &= 0xF7; break;
#pragma endregion

#pragma region CB instructions 0xA0 - 0xAF
		// RES 4, B
		case 0xA0: registers_.bc.High() &= 0xEF; break;
		// RES 4, C
		case 0xA1: registers_.bc.Low() &= 0xEF; break;
		// RES 4, D
		case 0xA2: registers_.de.High() &= 0xEF; break;
		// RES 4, E
		case 0xA3: registers_.de.Low() &= 0xEF; break;
		// RES 4, H
		case 0xA4: registers_.hl.High() &= 0xEF; break;
		// RES 4, L
		case 0xA5: registers_.hl.Low() &= 0xEF; break;
		// RES 4, (HL)
		case 0xA6: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xEF); break;
		// RES 4, A
		case 0xA7: registers_.af.High() &= 0xEF; break;
		// RES 5, B
		case 0xA8: registers_.bc.High() &= 0xDF; break;
		// RES 5, C
		case 0xA9: registers_.bc.Low() &= 0xDF; break;
		// RES 5, D
		case 0xAA: registers_.de.High() &= 0xDF; break;
		// RES 5, E
		case 0xAB: registers_.de.Low() &= 0xDF; break;
		// RES 5, H
		case 0xAC: registers_.hl.High() &= 0xDF; break;
		// RES 5, L
		case 0xAD: registers_.hl.Low() &= 0xDF; break;
		// RES 5, (HL)
		case 0xAE: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xDF); break;
		// RES 5, A
		case 0xAF: registers_.af.High() &= 0xDF; break;
#pragma endregion

#pragma region CB instructions 0xB0 - 0xBF
		// RES 6, B
		case 0xB0: registers_.bc.High() &= 0xBF; break;
		// RES 6, C
		case 0xB1: registers_.bc.Low() &= 0xBF; break;
		// RES 6, D
		case 0xB2: registers_.de.High() &= 0xBF; break;
		// RES 6, E
		case 0xB3: registers_.de.Low() &= 0xBF; break;
		// RES 6, H
		case 0xB4: registers_.hl.High() &= 0xBF; break;
		// RES 6, L
		case 0xB5: registers_.hl.Low() &= 0xBF; break;
		// RES 6, (HL)
		case 0xB6: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0xBF); break;
		// RES 6, A
		case 0xB7: registers_.af.High() &= 0xBF; break;
		// RES 7, B
		case 0xB8: registers_.bc.High() &= 0x7F; break;
		// RES 7, C
		case 0xB9: registers_.bc.Low() &= 0x7F; break;
		// RES 7, D
		case 0xBA: registers_.de.High() &= 0x7F; break;
		// RES 7, E
		case 0xBB: registers_.de.Low() &= 0x7F; break;
		// RES 7, H
		case 0xBC: registers_.hl.High() &= 0x7F; break;
		// RES 7, L
		case 0xBD: registers_.hl.Low() &= 0x7F; break;
		// RES 7, (HL)
		case 0xBE: WriteByte(registers_.hl, ReadByte(registers_.hl) & 0x7F); break;
		// RES 7, A
		case 0xBF: registers_.af.High() &= 0x7F; break;
#pragma endregion

#pragma region CB instructions 0xC0 - 0xCF
		// SET 0, B
		case 0xC0: registers_.bc.High() |= 0x01; break;
		// SET 0, C
		case 0xC1: registers_.bc.Low() |= 0x01; break;
		// SET 0, D
		case 0xC2: registers_.de.High() |= 0x01; break;
		// SET 0, E
		case 0xC3: registers_.de.Low() |= 0x01; break;
		// SET 0, H
		case 0xC4: registers_.hl.High() |= 0x01; break;
		// SET 0, L
		case 0xC5: registers_.hl.Low() |= 0x01; break;
		// SET 0, (HL)
		case 0xC6: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x01); break;
		// SET 0, A
		case 0xC7: registers_.af.High() |= 0x01; break;
		// SET 1, B
		case 0xC8: registers_.bc.High() |= 0x02; break;
		// SET 1, C
		case 0xC9: registers_.bc.Low() |= 0x02; break;
		// SET 1, D
		case 0xCA: registers_.de.High() |= 0x02; break;
		// SET 1, E
		case 0xCB: registers_.de.Low() |= 0x02; break;
		// SET 1, H
		case 0xCC: registers_.hl.High() |= 0x02; break;
		// SET 1, L
		case 0xCD: registers_.hl.Low() |= 0x02; break;
		// SET 1, (HL)
		case 0xCE: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x02); break;
		// SET 1, A
		case 0xCF: registers_.af.High() |= 0x02; break;
#pragma endregion

#pragma region CB instructions 0xD0 - 0xDF
		// SET 2, B
		case 0xD0: registers_.bc.High() |= 0x04; break;
		// SET 2, C
		case 0xD1: registers_.bc.Low() |= 0x04; break;
		// SET 2, D
		case 0xD2: registers_.de.High() |= 0x04; break;
		// SET 2, E
		case 0xD3: registers_.de.Low() |= 0x04; break;
		// SET 2, H
		case 0xD4: registers_.hl.High() |= 0x04; break;
		// SET 2, L
		case 0xD5: registers_.hl.Low() |= 0x04; break;
		// SET 2, (HL)
		case 0xD6: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x04); break;
		// SET 2, A
		case 0xD7: registers_.af.High() |= 0x04; break;
		// SET 3, B
		case 0xD8: registers_.bc.High() |= 0x08; break;
		// SET 3, C
		case 0xD9: registers_.bc.Low() |= 0x08; break;
		// SET 3, D
		case 0xDA: registers_.de.High() |= 0x08; break;
		// SET 3, E
		case 0xDB: registers_.de.Low() |= 0x08; break;
		// SET 3, H
		case 0xDC: registers_.hl.High() |= 0x08; break;
		// SET 3, L
		case 0xDD: registers_.hl.Low() |= 0x08; break;
		// SET 3, (HL)
		case 0xDE: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x08); break;
		// SET 3, A
		case 0xDF: registers_.af.High() |= 0x08; break;
#pragma endregion

#pragma region CB instructions 0xE0 - 0xEF
		// SET 4, B
		case 0xE0: registers_.bc.High() |= 0x10; break;
		// SET 4, C
		case 0xE1: registers_.bc.Low() |= 0x10; break;
		// SET 4, D
		case 0xE2: registers_.de.High() |= 0x10; break;
		// SET 4, E
		case 0xE3: registers_.de.Low() |= 0x10; break;
		// SET 4, H
		case 0xE4: registers_.hl.High() |= 0x10; break;
		// SET 4, L
		case 0xE5: registers_.hl.Low() |= 0x10; break;
		// SET 4, (HL)
		case 0xE6: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x10); break;
		// SET 4, A
		case 0xE7: registers_.af.High() |= 0x10; break;
		// SET 5, B
		case 0xE8: registers_.bc.High() |= 0x20; break;
		// SET 5, C
		case 0xE9: registers_.bc.Low() |= 0x20; break;
		// SET 5, D
		case 0xEA: registers_.de.High() |= 0x20; break;
		// SET 5, E
		case 0xEB: registers_.de.Low() |= 0x20; break;
		// SET 5, H
		case 0xEC: registers_.hl.High() |= 0x20; break;
		// SET 5, L
		case 0xED: registers_.hl.Low() |= 0x20; break;
		// SET 5, (HL)
		case 0xEE: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x20); break;
		// SET 5, A
		case 0xEF: registers_.af.High() |= 0x20; break;
#pragma endregion

#pragma region CB instructions 0xF0 - 0xFF
		// SET 6, B
		case 0xF0: registers_.bc.High() |= 0x40; break;
		// SET 6, C
		case 0xF1: registers_.bc.Low() |= 0x40; break;
		// SET 6, D
		case 0xF2: registers_.de.High() |= 0x40; break;
		// SET 6, E
		case 0xF3: registers_.de.Low() |= 0x40; break;
		// SET 6, H
		case 0xF4: registers_.hl.High() |= 0x40; break;
		// SET 6, L
		case 0xF5: registers_.hl.Low() |= 0x40; break;
		// SET 6, (HL)
		case 0xF6: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x40); break;
		// SET 6, A
		case 0xF7: registers_.af.High() |= 0x40; break;
		// SET 7, B
		case 0xF8: registers_.bc.High() |= 0x80; break;
		// SET 7, C
		case 0xF9: registers_.bc.Low() |= 0x80; break;
		// SET 7, D
		case 0xFA: registers_.de.High() |= 0x80; break;
		// SET 7, E
		case 0xFB: registers_.de.Low() |= 0x80; break;
		// SET 7, H
		case 0xFC: registers_.hl.High() |= 0x80; break;
		// SET 7, L
		case 0xFD: registers_.hl.Low() |= 0x80; break;
		// SET 7, (HL)
		case 0xFE: WriteByte(registers_.hl, ReadByte(registers_.hl) | 0x80); break;
		// SET 7, A
		case 0xFF: registers_.af.High() |= 0x80; break;
#pragma endregion
	}
}
