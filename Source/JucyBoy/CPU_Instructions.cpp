#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"
#include "InstructionMnemonics.h"

void CPU::ExecuteInstruction(OpCode opcode)
{
	switch (opcode)
	{
#pragma region Instructions 0x00 - 0x0F
	// NOP
	case 0x00: break;
	// LD BC, d16
	case 0x01: registers_.bc = FetchWord(); break;
	// LD (BC), A
	case 0x02: WriteByte(registers_.bc, registers_.af.High()); break;
	// INC BC
	case 0x03: registers_.bc += 1; NotifyMachineCycleLapse(); break;
	// INC B
	case 0x04: registers_.bc.High() = IncrementRegister(registers_.bc.High()); break;
	// DEC B
	case 0x05: registers_.bc.High() = DecrementRegister(registers_.bc.High()); break;
	// LD B, d8
	case 0x06: registers_.bc.High() = FetchByte(); break;
	// RLCA
	case 0x07:
	{ ClearFlag(Flags::All);
	if ((registers_.af.High() & 0x80) != 0) SetFlag(Flags::C);
	registers_.af.High() = (registers_.af.High() << 1) | ((registers_.af.High() & 0x80) >> 7); }
	break;
	// LD (a16), SP
	case 0x08:
	{ Memory::Address address{ FetchWord() };
	WriteByte(address, registers_.sp & 0xFF);
	WriteByte(++address, (registers_.sp >> 8) & 0xFF); }
	break;
	// ADD HL, BC
	case 0x09: AddToHl(registers_.bc); NotifyMachineCycleLapse(); break;
	// LD A, (BC)
	case 0x0A: registers_.af.High() = ReadByte(registers_.bc); break;
	// DEC BC
	case 0x0B: registers_.bc -= 1; NotifyMachineCycleLapse(); break;
	// INC C
	case 0x0C: registers_.bc.Low() = IncrementRegister(registers_.bc.Low()); break;
	// DEC C
	case 0x0D: registers_.bc.Low() = DecrementRegister(registers_.bc.Low()); break;
	// LD C, d8
	case 0x0E: registers_.bc.Low() = FetchByte(); break;
	// RRCA
	case 0x0F:
	{ClearFlag(Flags::All);
	if ((registers_.af.High() & 0x01) != 0) SetFlag(Flags::C);
	registers_.af.High() = (registers_.af.High() >> 1) | ((registers_.af.High() & 0x01) << 7); }
	break;
#pragma endregion

#pragma region Instructions 0x10 - 0x1F
	//TODO STOP 0x10

	// LD DE, d16
	case 0x11: registers_.de = FetchWord(); break;
	// LD (DE), A
	case 0x12: WriteByte(registers_.de, registers_.af.High()); break;
	// INC DE
	case 0x13: registers_.de += 1; NotifyMachineCycleLapse(); break;
	// INC D
	case 0x14: registers_.de.High() = IncrementRegister(registers_.de.High()); break;
	// DEC D
	case 0x15: registers_.de.High() = DecrementRegister(registers_.de.High()); break;
	// LD D, d8
	case 0x16: registers_.de.High() = FetchByte(); break;
	// RLA
	case 0x17:
	{const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
	ClearFlag(Flags::All);
	if ((registers_.af.High() & 0x80) != 0) SetFlag(Flags::C);
	registers_.af.High() = (registers_.af.High() << 1) | carry_value; }
	break;
	// JR r8
	case 0x18: registers_.pc += static_cast<int8_t>(FetchByte()); NotifyMachineCycleLapse(); break;
	// ADD HL, DE
	case 0x19: AddToHl(registers_.de); NotifyMachineCycleLapse(); break;
	// LD A, (DE)
	case 0x1A: registers_.af.High() = ReadByte(registers_.de); break;
	// DEC DE
	case 0x1B: registers_.de -= 1; NotifyMachineCycleLapse(); break;
	// INC E
	case 0x1C: registers_.de.Low() = IncrementRegister(registers_.de.Low()); break;
	// DEC E
	case 0x1D: registers_.de.Low() = DecrementRegister(registers_.de.Low()); break;
	// LD E, d8
	case 0x1E: registers_.de.Low() = FetchByte(); break;
	// RRA
	case 0x1F:
	{const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
	ClearFlag(Flags::All);
	if ((registers_.af.High() & 0x01) != 0) SetFlag(Flags::C);
	registers_.af.High() >>= 1;
	registers_.af.High() |= (carry_value << 7); }
	break;
#pragma endregion

#pragma region Instructions 0x20 - 0x2F
	// JR NZ, r8
	case 0x20:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	if (!IsFlagSet(Flags::Z))
	{
		registers_.pc += displacement;
		NotifyMachineCycleLapse();
	}}
	break;

	// LD HL, d16
	case 0x21: registers_.hl = FetchWord(); break;
	// LD (HL+), A
	case 0x22: WriteByte(registers_.hl++, registers_.af.High()); break;
	// INC HL
	case 0x23: registers_.hl += 1; NotifyMachineCycleLapse(); break;
	// INC H
	case 0x24: registers_.hl.High() = IncrementRegister(registers_.hl.High()); break;
	// DEC H
	case 0x25: registers_.hl.High() = DecrementRegister(registers_.hl.High()); break;
	// LD H, d8
	case 0x26: registers_.hl.High() = FetchByte(); break;
	// DAA
	case 0x27:
	{uint8_t correction_factor{ 0 };
	const auto original_value = registers_.af.High();

	// Correct the high nibble first
	if (((original_value > 0x99) && (!IsFlagSet(Flags::N))) || IsFlagSet(Flags::C))
	{
		correction_factor |= 0x60;
		SetFlag(Flags::C);
	}

	// Correct the low nibble
	if ((((original_value & 0x0F) > 9) && (!IsFlagSet(Flags::N))) || IsFlagSet(Flags::H))
	{
		correction_factor |= 0x06;
	}
	ClearFlag(Flags::H);

	const uint8_t result = IsFlagSet(Flags::N) ? original_value - correction_factor :
		original_value + correction_factor;

	(result == 0) ? SetFlag(Flags::Z) : ClearFlag(Flags::Z);

	registers_.af.High() = result;
	}
	break;
	// JR Z, r8
	case 0x28:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	if (IsFlagSet(Flags::Z))
	{
		registers_.pc += displacement;
		NotifyMachineCycleLapse();
	}}
	break;
	// ADD HL, HL
	case 0x29: AddToHl(registers_.hl); NotifyMachineCycleLapse(); break;
	// LD A, (HL+)
	case 0x2A: registers_.af.High() = ReadByte(registers_.hl++); break;
	// DEC HL
	case 0x2B: registers_.hl -= 1; NotifyMachineCycleLapse(); break;
	// INC L
	case 0x2C: registers_.hl.Low() = IncrementRegister(registers_.hl.Low()); break;
	// DEC L
	case 0x2D: registers_.hl.Low() = DecrementRegister(registers_.hl.Low()); break;
	// LD L, d8
	case 0x2E: registers_.hl.Low() = FetchByte(); break;
	// CPL
	case 0x2F: registers_.af.High() ^= 0xFF; SetFlag(Flags::H | Flags::N); break;
#pragma endregion

#pragma region Instructions 0x30 - 0x3F
	// JR NC, r8
	case 0x30:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	if (!IsFlagSet(Flags::C))
	{
		registers_.pc += displacement;
		NotifyMachineCycleLapse();
	}}
	break;
	// LD SP, d16
	case 0x31: registers_.sp = FetchWord(); break;
	// LD (HL-), A
	case 0x32: WriteByte(registers_.hl--, registers_.af.High()); break;
	// INC SP
	case 0x33: registers_.sp += 1; NotifyMachineCycleLapse(); break;
	// INC (HL)
	case 0x34:
	{ClearFlag(Flags::Z | Flags::H | Flags::N);
	auto value = ReadByte(registers_.hl);
	WriteByte(registers_.hl, ++value);
	if ((value & 0x0F) == 0x00) { SetFlag(Flags::H); }
	if (value == 0) { SetFlag(Flags::Z); }}
	break;
	// DEC (HL)
	case 0x35:
	{ClearFlag(Flags::Z | Flags::H);
	SetFlag(Flags::N);
	auto value = ReadByte(registers_.hl);
	WriteByte(registers_.hl, --value);
	if ((value & 0x0F) == 0x0F) { SetFlag(Flags::H); }
	if (value == 0) { SetFlag(Flags::Z); }}
	break;
	// LD (HL), d8
	case 0x36: WriteByte(registers_.hl, FetchByte()); break;
	// SCF
	case 0x37: SetFlag(Flags::C); ClearFlag(Flags::H | Flags::N); break;
	// JR C, r8
	case 0x38:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	if (IsFlagSet(Flags::C))
	{
		registers_.pc += displacement;
		NotifyMachineCycleLapse();
	}}
	break;
	// ADD HL, SP
	case 0x39: AddToHl(registers_.sp); NotifyMachineCycleLapse(); break;
	// LD A, (HL-)
	case 0x3A: registers_.af.High() = ReadByte(registers_.hl--); break;
	// DEC SP
	case 0x3B: registers_.sp -= 1; NotifyMachineCycleLapse(); break;
	// INC A
	case 0x3C: registers_.af.High() = IncrementRegister(registers_.af.High()); break;
	// DEC A
	case 0x3D: registers_.af.High() = DecrementRegister(registers_.af.High()); break;
	// LD A, d8
	case 0x3E: registers_.af.High() = FetchByte(); break;
	// CCF
	case 0x3F: ToggleFlag(Flags::C); ClearFlag(Flags::H | Flags::N); break;
#pragma endregion

#pragma region Instructions 0x40 - 0x4F
	// LD B, B
	case 0x40: break;
	// LD B, C
	case 0x41: registers_.bc.High() = registers_.bc.Low(); break;
	// LD B, D
	case 0x42: registers_.bc.High() = registers_.de.High(); break;
	// LD B, E
	case 0x43: registers_.bc.High() = registers_.de.Low(); break;
	// LD B, H
	case 0x44: registers_.bc.High() = registers_.hl.High(); break;
	// LD B, L
	case 0x45: registers_.bc.High() = registers_.hl.Low(); break;
	// LD B, (HL)
	case 0x46: registers_.bc.High() = ReadByte(registers_.hl); break;
	// LD B, A
	case 0x47: registers_.bc.High() = registers_.af.High(); break;
	// LD C, B
	case 0x48: registers_.bc.Low() = registers_.bc.High(); break;
	// LD C, C
	case 0x49: break;
	// LD C, D
	case 0x4A: registers_.bc.Low() = registers_.de.High(); break;
	// LD C, E
	case 0x4B: registers_.bc.Low() = registers_.de.Low(); break;
	// LD C, H
	case 0x4C: registers_.bc.Low() = registers_.hl.High(); break;
	// LD C, L
	case 0x4D: registers_.bc.Low() = registers_.hl.Low(); break;
	// LD C, (HL)
	case 0x4E: registers_.bc.Low() = ReadByte(registers_.hl); break;
	// LD C, A
	case 0x4F: registers_.bc.Low() = registers_.af.High(); break;
#pragma endregion

#pragma region Instructions 0x50 - 0x5F
	// LD D, B
	case 0x50: registers_.de.High() = registers_.bc.High(); break;
	// LD D, C
	case 0x51: registers_.de.High() = registers_.bc.Low(); break;
	// LD D, D
	case 0x52: break;
	// LD D, E
	case 0x53: registers_.de.High() = registers_.de.Low(); break;
	// LD D, H
	case 0x54: registers_.de.High() = registers_.hl.High(); break;
	// LD D, L
	case 0x55: registers_.de.High() = registers_.hl.Low(); break;
	// LD D, (HL)
	case 0x56: registers_.de.High() = ReadByte(registers_.hl); break;
	// LD D, A
	case 0x57: registers_.de.High() = registers_.af.High(); break;
	// LD E, B
	case 0x58: registers_.de.Low() = registers_.bc.High(); break;
	// LD E, C
	case 0x59: registers_.de.Low() = registers_.bc.Low(); break;
	// LD E, D
	case 0x5A: registers_.de.Low() = registers_.de.High(); break;
	// LD E, E
	case 0x5B: break;
	// LD E, H
	case 0x5C: registers_.de.Low() = registers_.hl.High(); break;
	// LD E, L
	case 0x5D: registers_.de.Low() = registers_.hl.Low(); break;
	// LD E, (HL)
	case 0x5E: registers_.de.Low() = ReadByte(registers_.hl); break;
	// LD E, A
	case 0x5F: registers_.de.Low() = registers_.af.High(); break;
#pragma endregion

#pragma region Instructions 0x60 - 0x6F
	// LD H, B
	case 0x60: registers_.hl.High() = registers_.bc.High(); break;
	// LD H, C
	case 0x61: registers_.hl.High() = registers_.bc.Low(); break;
	// LD H, D
	case 0x62: registers_.hl.High() = registers_.de.High(); break;
	// LD H, E
	case 0x63: registers_.hl.High() = registers_.de.Low(); break;
	// LD H, H
	case 0x64: break;
	// LD H, L
	case 0x65: registers_.hl.High() = registers_.hl.Low(); break;
	// LD H, (HL)
	case 0x66: registers_.hl.High() = ReadByte(registers_.hl); break;
	// LD H, A
	case 0x67: registers_.hl.High() = registers_.af.High(); break;
	// LD L, B
	case 0x68: registers_.hl.Low() = registers_.bc.High(); break;
	// LD L, C
	case 0x69: registers_.hl.Low() = registers_.bc.Low(); break;
	// LD L, D
	case 0x6A: registers_.hl.Low() = registers_.de.High(); break;
	// LD L, E
	case 0x6B: registers_.hl.Low() = registers_.de.Low(); break;
	// LD L, H
	case 0x6C: registers_.hl.Low() = registers_.hl.High(); break;
	// LD L, L
	case 0x6D: break;
	// LD L, (HL)
	case 0x6E: registers_.hl.Low() = ReadByte(registers_.hl); break;
	// LD L, A
	case 0x6F: registers_.hl.Low() = registers_.af.High(); break;
#pragma endregion

#pragma region Instructions 0x70 - 0x7F
	// LD (HL), B
	case 0x70: WriteByte(registers_.hl, registers_.bc.High()); break;
	// LD (HL), C
	case 0x71: WriteByte(registers_.hl, registers_.bc.Low()); break;
	// LD (HL), D
	case 0x72: WriteByte(registers_.hl, registers_.de.High()); break;
	// LD (HL), E
	case 0x73: WriteByte(registers_.hl, registers_.de.Low()); break;
	// LD (HL), H
	case 0x74: WriteByte(registers_.hl, registers_.hl.High()); break;
	// LD (HL), L
	case 0x75: WriteByte(registers_.hl, registers_.hl.Low()); break;
	// HALT
	case 0x76: current_state_ = (interrupt_master_enable_ || ((enabled_interrupts_ & requested_interrupts_) == 0)) ? State::Halted : State::HaltBug; break;
	// LD (HL), A
	case 0x77: WriteByte(registers_.hl, registers_.af.High()); break;
	// LD A, B
	case 0x78: registers_.af.High() = registers_.bc.High(); break;
	// LD A, C
	case 0x79: registers_.af.High() = registers_.bc.Low(); break;
	// LD A, D
	case 0x7A: registers_.af.High() = registers_.de.High(); break;
	// LD A, E
	case 0x7B: registers_.af.High() = registers_.de.Low(); break;
	// LD A, H
	case 0x7C: registers_.af.High() = registers_.hl.High(); break;
	// LD A, L
	case 0x7D: registers_.af.High() = registers_.hl.Low(); break;
	// LD A, (HL)
	case 0x7E: registers_.af.High() = ReadByte(registers_.hl); break;
	// LD A, A
	case 0x7F: break;
#pragma endregion

#pragma region Instructions 0x80 - 0x8F
	// ADD A, B
	case 0x80: Add(registers_.bc.High()); break;
	// ADD A, C
	case 0x81: Add(registers_.bc.Low()); break;
	// ADD A, D
	case 0x82: Add(registers_.de.High()); break;
	// ADD A, E
	case 0x83: Add(registers_.de.Low()); break;
	// ADD A, H
	case 0x84: Add(registers_.hl.High()); break;
	// ADD A, L
	case 0x85: Add(registers_.hl.Low()); break;
	// ADD (HL)
	case 0x86: Add(ReadByte(registers_.hl)); break;
	// ADD A, A
	case 0x87: Add(registers_.af.High()); break;
	// ADC A, B
	case 0x88: Adc(registers_.bc.High()); break;
	// ADC A, C
	case 0x89: Adc(registers_.bc.Low()); break;
	// ADC A, D
	case 0x8A: Adc(registers_.de.High()); break;
	// ADC A, E
	case 0x8B: Adc(registers_.de.Low()); break;
	// ADC A, H
	case 0x8C: Adc(registers_.hl.High()); break;
	// ADC A, L
	case 0x8D: Adc(registers_.hl.Low()); break;
	// ADC A, (HL)
	case 0x8E: Adc(ReadByte(registers_.hl)); break;
	// ADC A, A
	case 0x8F: Adc(registers_.af.High()); break;
#pragma endregion

#pragma region Instructions 0x90 - 0x9F
	// SUB B
	case 0x90: Sub(registers_.bc.High()); break;
	// SUB C
	case 0x91: Sub(registers_.bc.Low()); break;
	// SUB D
	case 0x92: Sub(registers_.de.High()); break;
	// SUB E
	case 0x93: Sub(registers_.de.Low()); break;
	// SUB H
	case 0x94: Sub(registers_.hl.High()); break;
	// SUB L
	case 0x95: Sub(registers_.hl.Low()); break;
	// SUB (HL)
	case 0x96: Sub(ReadByte(registers_.hl)); break;
	// SUB A
	case 0x97: Sub(registers_.af.High()); break;
	// SBC A, B
	case 0x98: Sbc(registers_.bc.High()); break;
	// SBC A, C
	case 0x99: Sbc(registers_.bc.Low()); break;
	// SBC A, D
	case 0x9A: Sbc(registers_.de.High()); break;
	// SBC A, E
	case 0x9B: Sbc(registers_.de.Low()); break;
	// SBC A, H
	case 0x9C: Sbc(registers_.hl.High()); break;
	// SBC A, L
	case 0x9D: Sbc(registers_.hl.Low()); break;
	// SBC A, (HL)
	case 0x9E: Sbc(ReadByte(registers_.hl)); break;
	// SBC A, A
	case 0x9F: Sbc(registers_.af.High()); break;
#pragma endregion

#pragma region Instructions 0xA0 - 0xAF
	// AND B
	case 0xA0: And(registers_.bc.High()); break;
	// AND C
	case 0xA1: And(registers_.bc.Low()); break;
	// AND D
	case 0xA2: And(registers_.de.High()); break;
	// AND E
	case 0xA3: And(registers_.de.Low()); break;
	// AND H
	case 0xA4: And(registers_.hl.High()); break;
	// AND L
	case 0xA5: And(registers_.hl.Low()); break;
	// AND (HL)
	case 0xA6: And(ReadByte(registers_.hl)); break;
	// AND A
	case 0xA7: And(registers_.af.High()); break;
	// XOR B
	case 0xA8: Xor(registers_.bc.High()); break;
	// XOR C
	case 0xA9: Xor(registers_.bc.Low()); break;
	// XOR D
	case 0xAA: Xor(registers_.de.High()); break;
	// XOR E
	case 0xAB: Xor(registers_.de.Low()); break;
	// XOR H
	case 0xAC: Xor(registers_.hl.High()); break;
	// XOR L
	case 0xAD: Xor(registers_.hl.Low()); break;
	// XOR (HL)
	case 0xAE: Xor(ReadByte(registers_.hl)); break;
	// XOR A
	case 0xAF: Xor(registers_.af.High()); break;
#pragma endregion

#pragma region Instructions 0xB0 - 0xBF
	// OR B
	case 0xB0: Or(registers_.bc.High()); break;
	// OR C
	case 0xB1: Or(registers_.bc.Low()); break;
	// OR D
	case 0xB2: Or(registers_.de.High()); break;
	// OR E
	case 0xB3: Or(registers_.de.Low()); break;
	// OR H
	case 0xB4: Or(registers_.hl.High()); break;
	// OR L
	case 0xB5: Or(registers_.hl.Low()); break;
	// OR (HL)
	case 0xB6: Or(ReadByte(registers_.hl)); break;
	// OR A
	case 0xB7: Or(registers_.af.High()); break;
	// CP B
	case 0xB8: Compare(registers_.bc.High()); break;
	// CP C
	case 0xB9: Compare(registers_.bc.Low()); break;
	// CP D
	case 0xBA: Compare(registers_.de.High()); break;
	// CP E
	case 0xBB: Compare(registers_.de.Low()); break;
	// CP H
	case 0xBC: Compare(registers_.hl.High()); break;
	// CP L
	case 0xBD: Compare(registers_.hl.Low()); break;
	// CP (HL)
	case 0xBE: Compare(ReadByte(registers_.hl)); break;
	// CP A
	case 0xBF: Compare(registers_.af.High()); break;
#pragma endregion

#pragma region Instructions 0xC0 - 0xCF
	// RET NZ
	case 0xC0: NotifyMachineCycleLapse(); if (!IsFlagSet(Flags::Z)) { Return(); } break;
	// POP BC
	case 0xC1: registers_.bc = PopWordFromStack(); break;
	// JP NZ, a16
	case 0xC2:
	{const auto address = FetchWord();
	if (!IsFlagSet(Flags::Z))
	{
		registers_.pc = address;
		NotifyMachineCycleLapse();
	}}
	break;
	// JP a16
	case 0xC3: registers_.pc = FetchWord(); NotifyMachineCycleLapse(); break;
	// CALL NZ, a16
	case 0xC4: { const auto address = FetchWord(); if (!IsFlagSet(Flags::Z)) { Call(address); } break; }
	// PUSH BC
	case 0xC5: PushWordToStack(registers_.bc); break;
	// ADD A, d8
	case 0xC6: Add(FetchByte()); break;
	// RST 00H
	case 0xC7: Call(0x0000); break;
	// RET Z
	case 0xC8: NotifyMachineCycleLapse(); if (IsFlagSet(Flags::Z)) { Return(); } break;
	// RET
	case 0xC9: Return(); break;
	// JP Z, a16
	case 0xCA:
	{const auto address = FetchWord();
	if (IsFlagSet(Flags::Z))
	{
		registers_.pc = address;
		NotifyMachineCycleLapse();
	}}
	break;
	// PREFIX CB
	case 0xCB: ExecuteCbInstruction(FetchByte()); break;
	// CALL Z, a16
	case 0xCC: { const auto address = FetchWord(); if (IsFlagSet(Flags::Z)) { Call(address); } break; }
	// CALL a16
	case 0xCD: Call(FetchWord()); break;
	// ADC A, d8
	case 0xCE: Adc(FetchByte()); break;
	// RST 08H
	case 0xCF: Call(0x0008); break;
#pragma endregion

#pragma region Instructions 0xD0 - 0xDF
	// RET NC
	case 0xD0:
	{NotifyMachineCycleLapse();
	if (!IsFlagSet(Flags::C))
	{
		Return();
	}}
	break;
	// POP DE
	case 0xD1: registers_.de = PopWordFromStack(); break;
	// JP NC, a16
	case 0xD2:
	{const auto address = FetchWord();
	if (!IsFlagSet(Flags::C))
	{
		registers_.pc = address;
		NotifyMachineCycleLapse();
	}}
	break;
	// CALL NC, a16
	case 0xD4:
	{const auto address = FetchWord();
	if (!IsFlagSet(Flags::C))
	{
		Call(address);
	}}
	break;
	// PUSH DE
	case 0xD5: PushWordToStack(registers_.de); break;
	// SUB A, d8
	case 0xD6: Sub(FetchByte()); break;
	// RST 10H
	case 0xD7: Call(0x0010); break;
	// RET C
	case 0xD8:
	{NotifyMachineCycleLapse();
	if (IsFlagSet(Flags::C))
	{
		Return();
	}}
	break;
	// RETI
	case 0xD9: Return(); interrupt_master_enable_ = true; break;
	// JP C, a16
	case 0xDA:
	{const auto address = FetchWord();
	if (IsFlagSet(Flags::C))
	{
		registers_.pc = address;
		NotifyMachineCycleLapse();
	}}
	break;
	// CALL C, a16
	case 0xDC:
	{const auto address = FetchWord();
	if (IsFlagSet(Flags::C))
	{
		Call(address);
	}}
	break;
	// SBC A, d8
	case 0xDE: Sbc(FetchByte()); break;
	// RST 18H
	case 0xDF: Call(0x0018); break;
#pragma endregion

#pragma region Instructions 0xE0 - 0xEF
	// LDH (a8), A
	case 0xE0: WriteByte(Memory::IO + FetchByte(), registers_.af.High()); break;
	// POP HL
	case 0xE1: registers_.hl = PopWordFromStack(); break;
	// LD (C), A
	case 0xE2: WriteByte(Memory::IO + registers_.bc.Low(), registers_.af.High()); break;
	// PUSH HL
	case 0xE5: PushWordToStack(registers_.hl); break;
	// AND d8
	case 0xE6: And(FetchByte()); break;
	// RST 20H
	case 0xE7: Call(0x0020); break;
	// ADD SP, r8
	case 0xE8:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	ClearFlag(Flags::All);
	// Flags are calculated on the lower byte of SP, adding an unsigned displacement
	if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
	if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
	registers_.sp += displacement;
	NotifyMachineCycleLapse();
	NotifyMachineCycleLapse(); }
	break;

	// JP (HL)
	case 0xE9: registers_.pc = registers_.hl; break;
	// LD (a16), A
	case 0xEA: WriteByte(FetchWord(), registers_.af.High()); break;
	// XOR d8
	case 0xEE: Xor(FetchByte()); break;
	// RST 28H
	case 0xEF: Call(0x0028); break;
#pragma endregion

#pragma region Instructions 0xF0 - 0xFF
	// LDH A, (a8)
	case 0xF0: registers_.af.High() = ReadByte(Memory::IO + FetchByte()); break;
	// POP AF
	case 0xF1: registers_.af = PopWordFromStack() & 0xFFF0; break;
	// LD A, (C)
	case 0xF2: registers_.af.High() = ReadByte(Memory::IO + registers_.bc.Low()); break;
	// DI
	case 0xF3: interrupt_master_enable_ = false; break;
	// PUSH AF
	case 0xF5: PushWordToStack(registers_.af); break;
	// OR d8
	case 0xF6: Or(FetchByte()); break;
	// RST 30H
	case 0xF7: Call(0x0030); break;
	// LD HL, SP+r8
	case 0xF8:
	{const auto displacement = static_cast<int8_t>(FetchByte());
	ClearFlag(Flags::All);
	// Flags are calculated on the lower byte of SP, adding an unsigned displacement
	if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
	if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
	registers_.hl = registers_.sp + displacement;
	NotifyMachineCycleLapse(); }
	break;
	// LD SP, HL
	case 0xF9: registers_.sp = registers_.hl; NotifyMachineCycleLapse(); break;
	// LD A, (a16)
	case 0xFA: registers_.af.High() = ReadByte(FetchWord()); break;
	// EI
	case 0xFB: if (!interrupt_master_enable_) ime_requested_ = true; break;
	// CP d8
	case 0xFE: Compare(FetchByte()); break;
	// RST 38H
	case 0xFF: Call(0x0038); break;
#pragma endregion
	}
}
