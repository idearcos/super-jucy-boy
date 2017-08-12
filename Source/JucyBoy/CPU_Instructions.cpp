#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"
#include "InstructionMnemonics.h"

void CPU::PopulateInstructions()
{
	for (int ii = 0; ii < instructions_.size(); ++ii)
	{
		instructions_[ii] = [this, ii]() {
			registers_.pc = previous_pc_;
			std::stringstream error;
			error << "Instruction not implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << GetInstructionMnemonic(ii) << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc << ")";
			throw std::runtime_error(error.str());
		};
	}

#pragma region Instructions 0x00 - 0x0F
	// NOP
	instructions_[0x00] = [this]() {};
	// LD BC, d16
	instructions_[0x01] = [this]() { registers_.bc = FetchWord(); };
	// LD (BC), A
	instructions_[0x02] = [this]() { WriteByte(registers_.bc, registers_.af.High()); };
	// INC BC
	instructions_[0x03] = [this]() { registers_.bc += 1; NotifyMachineCycleLapse(); }; 
	// INC B
	instructions_[0x04] = [this]() { registers_.bc.High() = IncrementRegister(registers_.bc.High()); };
	// DEC B
	instructions_[0x05] = [this]() { registers_.bc.High() = DecrementRegister(registers_.bc.High()); };
	// LD B, d8
	instructions_[0x06] = [this]() { registers_.bc.High() = FetchByte(); };
	// RLCA
	instructions_[0x07] = [this]() {
		ClearFlag(Flags::All);
		if ((registers_.af.High() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.High() = (registers_.af.High() << 1) | ((registers_.af.High() & 0x80) >> 7);
	};
	// LD (a16), SP
	instructions_[0x08] = [this]() {
		Memory::Address address{ FetchWord() };
		WriteByte(address, registers_.sp & 0xFF);
		WriteByte(++address, (registers_.sp >> 8) & 0xFF);
	};
	// ADD HL, BC
	instructions_[0x09] = [this]() { AddToHl(registers_.bc); NotifyMachineCycleLapse(); }; 
	// LD A, (BC)
	instructions_[0x0A] = [this]() { registers_.af.High() = ReadByte(registers_.bc); }; 
	// DEC BC
	instructions_[0x0B] = [this]() { registers_.bc -= 1; NotifyMachineCycleLapse(); };
	// INC C
	instructions_[0x0C] = [this]() { registers_.bc.Low() = IncrementRegister(registers_.bc.Low()); };
	// DEC C
	instructions_[0x0D] = [this]() { registers_.bc.Low() = DecrementRegister(registers_.bc.Low()); };
	// LD C, d8
	instructions_[0x0E] = [this]() { registers_.bc.Low() = FetchByte(); };
	// RRCA
	instructions_[0x0F] = [this]() {
		ClearFlag(Flags::All);
		if ((registers_.af.High() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.High() = (registers_.af.High() >> 1) | ((registers_.af.High() & 0x01) << 7);
	};
#pragma endregion

#pragma region Instructions 0x10 - 0x1F
	//TODO STOP 0x10

	// LD DE, d16
	instructions_[0x11] = [this]() { registers_.de = FetchWord(); };
	// LD (DE), A
	instructions_[0x12] = [this]() { WriteByte(registers_.de, registers_.af.High()); };
	// INC DE
	instructions_[0x13] = [this]() { registers_.de += 1; NotifyMachineCycleLapse(); };
	// INC D
	instructions_[0x14] = [this]() { registers_.de.High() = IncrementRegister(registers_.de.High()); };
	// DEC D
	instructions_[0x15] = [this]() { registers_.de.High() = DecrementRegister(registers_.de.High()); };
	// LD D, d8
	instructions_[0x16] = [this]() { registers_.de.High() = FetchByte(); };
	// RLA
	instructions_[0x17] = [this]() {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.High() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.High() = (registers_.af.High() << 1) | carry_value;
	};
	// JR r8
	instructions_[0x18] = [this]() { registers_.pc += static_cast<int8_t>(FetchByte()); NotifyMachineCycleLapse(); };
	// ADD HL, DE
	instructions_[0x19] = [this]() { AddToHl(registers_.de); NotifyMachineCycleLapse(); };
	// LD A, (DE)
	instructions_[0x1A] = [this]() { registers_.af.High() = ReadByte(registers_.de); };
	// DEC DE
	instructions_[0x1B] = [this]() { registers_.de -= 1; NotifyMachineCycleLapse(); };
	// INC E
	instructions_[0x1C] = [this]() { registers_.de.Low() = IncrementRegister(registers_.de.Low()); };
	// DEC E
	instructions_[0x1D] = [this]() { registers_.de.Low() = DecrementRegister(registers_.de.Low()); };
	// LD E, d8
	instructions_[0x1E] = [this]() { registers_.de.Low() = FetchByte(); };
	// RRA
	instructions_[0x1F] = [this]() {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.High() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.High() >>= 1;
		registers_.af.High() |= (carry_value << 7);
	};
#pragma endregion

#pragma region Instructions 0x20 - 0x2F
	// JR NZ, r8
	instructions_[0x20] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (!IsFlagSet(Flags::Z))
		{
			registers_.pc += displacement;
			NotifyMachineCycleLapse();
		}
	};

	// LD HL, d16
	instructions_[0x21] = [this]() { registers_.hl = FetchWord(); };
	// LD (HL+), A
	instructions_[0x22] = [this]() { WriteByte(registers_.hl++, registers_.af.High()); };
	// INC HL
	instructions_[0x23] = [this]() { registers_.hl += 1; NotifyMachineCycleLapse(); };
	// INC H
	instructions_[0x24] = [this]() { registers_.hl.High() = IncrementRegister(registers_.hl.High()); };
	// DEC H
	instructions_[0x25] = [this]() { registers_.hl.High() = DecrementRegister(registers_.hl.High()); };
	// LD H, d8
	instructions_[0x26] = [this]() { registers_.hl.High() = FetchByte(); };
	// DAA
	instructions_[0x27] = [this]() {
		uint8_t correction_factor{ 0 };
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
	};
	// JR Z, r8
	instructions_[0x28] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (IsFlagSet(Flags::Z))
		{
			registers_.pc += displacement;
			NotifyMachineCycleLapse();
		}
	};
	// ADD HL, HL
	instructions_[0x29] = [this]() { AddToHl(registers_.hl); NotifyMachineCycleLapse(); };
	// LD A, (HL+)
	instructions_[0x2A] = [this]() { registers_.af.High() = ReadByte(registers_.hl++); };
	// DEC HL
	instructions_[0x2B] = [this]() { registers_.hl -= 1; NotifyMachineCycleLapse(); };
	// INC L
	instructions_[0x2C] = [this]() { registers_.hl.Low() = IncrementRegister(registers_.hl.Low()); };
	// DEC L
	instructions_[0x2D] = [this]() { registers_.hl.Low() = DecrementRegister(registers_.hl.Low()); };
	// LD L, d8
	instructions_[0x2E] = [this]() { registers_.hl.Low() = FetchByte(); };
	// CPL
	instructions_[0x2F] = [this]() { registers_.af.High() ^= 0xFF; SetFlag(Flags::H | Flags::N); };
#pragma endregion

#pragma region Instructions 0x30 - 0x3F
	// JR NC, r8
	instructions_[0x30] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (!IsFlagSet(Flags::C))
		{
			registers_.pc += displacement;
			NotifyMachineCycleLapse();
		}
	};
	// LD SP, d16
	instructions_[0x31] = [this]() { registers_.sp = FetchWord(); };
	// LD (HL-), A
	instructions_[0x32] = [this]() { WriteByte(registers_.hl--, registers_.af.High()); };
	// INC SP
	instructions_[0x33] = [this]() { registers_.sp += 1; NotifyMachineCycleLapse(); };
	// INC (HL)
	instructions_[0x34] = [this]() {
		ClearFlag(Flags::Z | Flags::H | Flags::N);
		auto value = ReadByte(registers_.hl);
		WriteByte(registers_.hl, ++value);
		if ((value & 0x0F) == 0x00) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }
	};
	// DEC (HL)
	instructions_[0x35] = [this]() {
		ClearFlag(Flags::Z | Flags::H);
		SetFlag(Flags::N);
		auto value = ReadByte(registers_.hl);
		WriteByte(registers_.hl, --value);
		if ((value & 0x0F) == 0x0F) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }
	};
	// LD (HL), d8
	instructions_[0x36] = [this]() { WriteByte(registers_.hl, FetchByte()); };
	// SCF
	instructions_[0x37] = [this]() { SetFlag(Flags::C); ClearFlag(Flags::H | Flags::N); };
	// JR C, r8
	instructions_[0x38] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (IsFlagSet(Flags::C))
		{
			registers_.pc += displacement;
			NotifyMachineCycleLapse();
		}
	};
	// ADD HL, SP
	instructions_[0x39] = [this]() { AddToHl(registers_.sp); NotifyMachineCycleLapse(); };
	// LD A, (HL-)
	instructions_[0x3A] = [this]() { registers_.af.High() = ReadByte(registers_.hl--); };
	// DEC SP
	instructions_[0x3B] = [this]() { registers_.sp -= 1; NotifyMachineCycleLapse(); };
	// INC A
	instructions_[0x3C] = [this]() { registers_.af.High() = IncrementRegister(registers_.af.High()); };
	// DEC A
	instructions_[0x3D] = [this]() { registers_.af.High() = DecrementRegister(registers_.af.High()); };
	// LD A, d8
	instructions_[0x3E] = [this]() { registers_.af.High() = FetchByte(); };
	// CCF
	instructions_[0x3F] = [this]() { ToggleFlag(Flags::C); ClearFlag(Flags::H | Flags::N); };
#pragma endregion

#pragma region Instructions 0x40 - 0x4F
	// LD B, B
	instructions_[0x40] = [this]() {};
	// LD B, C
	instructions_[0x41] = [this]() { registers_.bc.High() = registers_.bc.Low(); };
	// LD B, D
	instructions_[0x42] = [this]() { registers_.bc.High() = registers_.de.High(); };
	// LD B, E
	instructions_[0x43] = [this]() { registers_.bc.High() = registers_.de.Low(); };
	// LD B, H
	instructions_[0x44] = [this]() { registers_.bc.High() = registers_.hl.High(); };
	// LD B, L
	instructions_[0x45] = [this]() { registers_.bc.High() = registers_.hl.Low(); };
	// LD B, (HL)
	instructions_[0x46] = [this]() { registers_.bc.High() = ReadByte(registers_.hl); };
	// LD B, A
	instructions_[0x47] = [this]() { registers_.bc.High() = registers_.af.High(); };
	// LD C, B
	instructions_[0x48] = [this]() { registers_.bc.Low() = registers_.bc.High(); };
	// LD C, C
	instructions_[0x49] = [this]() {};
	// LD C, D
	instructions_[0x4A] = [this]() { registers_.bc.Low() = registers_.de.High(); };
	// LD C, E
	instructions_[0x4B] = [this]() { registers_.bc.Low() = registers_.de.Low(); };
	// LD C, H
	instructions_[0x4C] = [this]() { registers_.bc.Low() = registers_.hl.High(); };
	// LD C, L
	instructions_[0x4D] = [this]() { registers_.bc.Low() = registers_.hl.Low(); };
	// LD C, (HL)
	instructions_[0x4E] = [this]() { registers_.bc.Low() = ReadByte(registers_.hl); };
	// LD C, A
	instructions_[0x4F] = [this]() { registers_.bc.Low() = registers_.af.High(); };
#pragma endregion

#pragma region Instructions 0x50 - 0x5F
	// LD D, B
	instructions_[0x50] = [this]() { registers_.de.High() = registers_.bc.High(); };
	// LD D, C
	instructions_[0x51] = [this]() { registers_.de.High() = registers_.bc.Low(); };
	// LD D, D
	instructions_[0x52] = [this]() {};
	// LD D, E
	instructions_[0x53] = [this]() { registers_.de.High() = registers_.de.Low(); };
	// LD D, H
	instructions_[0x54] = [this]() { registers_.de.High() = registers_.hl.High(); };
	// LD D, L
	instructions_[0x55] = [this]() { registers_.de.High() = registers_.hl.Low(); };
	// LD D, (HL)
	instructions_[0x56] = [this]() { registers_.de.High() = ReadByte(registers_.hl); };
	// LD D, A
	instructions_[0x57] = [this]() { registers_.de.High() = registers_.af.High(); };
	// LD E, B
	instructions_[0x58] = [this]() { registers_.de.Low() = registers_.bc.High(); };
	// LD E, C
	instructions_[0x59] = [this]() { registers_.de.Low() = registers_.bc.Low(); };
	// LD E, D
	instructions_[0x5A] = [this]() { registers_.de.Low() = registers_.de.High(); };
	// LD E, E
	instructions_[0x5B] = [this]() {};
	// LD E, H
	instructions_[0x5C] = [this]() { registers_.de.Low() = registers_.hl.High(); };
	// LD E, L
	instructions_[0x5D] = [this]() { registers_.de.Low() = registers_.hl.Low(); };
	// LD E, (HL)
	instructions_[0x5E] = [this]() { registers_.de.Low() = ReadByte(registers_.hl); };
	// LD E, A
	instructions_[0x5F] = [this]() { registers_.de.Low() = registers_.af.High(); };
#pragma endregion

#pragma region Instructions 0x60 - 0x6F
	// LD H, B
	instructions_[0x60] = [this]() { registers_.hl.High() = registers_.bc.High(); };
	// LD H, C
	instructions_[0x61] = [this]() { registers_.hl.High() = registers_.bc.Low(); };
	// LD H, D
	instructions_[0x62] = [this]() { registers_.hl.High() = registers_.de.High(); };
	// LD H, E
	instructions_[0x63] = [this]() { registers_.hl.High() = registers_.de.Low(); };
	// LD H, H
	instructions_[0x64] = [this]() {};
	// LD H, L
	instructions_[0x65] = [this]() { registers_.hl.High() = registers_.hl.Low(); };
	// LD H, (HL)
	instructions_[0x66] = [this]() { registers_.hl.High() = ReadByte(registers_.hl); };
	// LD H, A
	instructions_[0x67] = [this]() { registers_.hl.High() = registers_.af.High(); };
	// LD L, B
	instructions_[0x68] = [this]() { registers_.hl.Low() = registers_.bc.High(); };
	// LD L, C
	instructions_[0x69] = [this]() { registers_.hl.Low() = registers_.bc.Low(); };
	// LD L, D
	instructions_[0x6A] = [this]() { registers_.hl.Low() = registers_.de.High(); };
	// LD L, E
	instructions_[0x6B] = [this]() { registers_.hl.Low() = registers_.de.Low(); };
	// LD L, H
	instructions_[0x6C] = [this]() { registers_.hl.Low() = registers_.hl.High(); };
	// LD L, L
	instructions_[0x6D] = [this]() {};
	// LD L, (HL)
	instructions_[0x6E] = [this]() { registers_.hl.Low() = ReadByte(registers_.hl); };
	// LD L, A
	instructions_[0x6F] = [this]() { registers_.hl.Low() = registers_.af.High(); };
#pragma endregion

#pragma region Instructions 0x70 - 0x7F
	// LD (HL), B
	instructions_[0x70] = [this]() { WriteByte(registers_.hl, registers_.bc.High()); };
	// LD (HL), C
	instructions_[0x71] = [this]() { WriteByte(registers_.hl, registers_.bc.Low()); };
	// LD (HL), D
	instructions_[0x72] = [this]() { WriteByte(registers_.hl, registers_.de.High()); };
	// LD (HL), E
	instructions_[0x73] = [this]() { WriteByte(registers_.hl, registers_.de.Low()); };
	// LD (HL), H
	instructions_[0x74] = [this]() { WriteByte(registers_.hl, registers_.hl.High()); };
	// LD (HL), L
	instructions_[0x75] = [this]() { WriteByte(registers_.hl, registers_.hl.Low()); };
	// HALT
	instructions_[0x76] = [this]() { current_state_ = (interrupt_master_enable_ || ((enabled_interrupts_ & requested_interrupts_) == 0)) ? State::Halted : State::HaltBug; };
	// LD (HL), A
	instructions_[0x77] = [this]() { WriteByte(registers_.hl, registers_.af.High()); };
	// LD A, B
	instructions_[0x78] = [this]() { registers_.af.High() = registers_.bc.High(); };
	// LD A, C
	instructions_[0x79] = [this]() { registers_.af.High() = registers_.bc.Low(); };
	// LD A, D
	instructions_[0x7A] = [this]() { registers_.af.High() = registers_.de.High(); };
	// LD A, E
	instructions_[0x7B] = [this]() { registers_.af.High() = registers_.de.Low(); };
	// LD A, H
	instructions_[0x7C] = [this]() { registers_.af.High() = registers_.hl.High(); };
	// LD A, L
	instructions_[0x7D] = [this]() { registers_.af.High() = registers_.hl.Low(); };
	// LD A, (HL)
	instructions_[0x7E] = [this]() { registers_.af.High() = ReadByte(registers_.hl); };
	// LD A, A
	instructions_[0x7F] = [this]() {};
#pragma endregion

#pragma region Instructions 0x80 - 0x8F
	// ADD A, B
	instructions_[0x80] = [this]() { Add(registers_.bc.High()); };
	// ADD A, C
	instructions_[0x81] = [this]() { Add(registers_.bc.Low()); }; 
	// ADD A, D
	instructions_[0x82] = [this]() { Add(registers_.de.High()); };
	// ADD A, E
	instructions_[0x83] = [this]() { Add(registers_.de.Low()); };
	// ADD A, H
	instructions_[0x84] = [this]() { Add(registers_.hl.High()); };
	// ADD A, L
	instructions_[0x85] = [this]() { Add(registers_.hl.Low()); };
	// ADD (HL)
	instructions_[0x86] = [this]() { Add(ReadByte(registers_.hl)); };
	// ADD A, A
	instructions_[0x87] = [this]() { Add(registers_.af.High()); };
	// ADC A, B
	instructions_[0x88] = [this]() { Adc(registers_.bc.High()); }; 
	// ADC A, C
	instructions_[0x89] = [this]() { Adc(registers_.bc.Low()); };
	// ADC A, D
	instructions_[0x8A] = [this]() { Adc(registers_.de.High()); };
	// ADC A, E
	instructions_[0x8B] = [this]() { Adc(registers_.de.Low()); };
	// ADC A, H
	instructions_[0x8C] = [this]() { Adc(registers_.hl.High()); };
	// ADC A, L
	instructions_[0x8D] = [this]() { Adc(registers_.hl.Low()); };
	// ADC A, (HL)
	instructions_[0x8E] = [this]() { Adc(ReadByte(registers_.hl)); };
	// ADC A, A
	instructions_[0x8F] = [this]() { Adc(registers_.af.High()); };
#pragma endregion

#pragma region Instructions 0x90 - 0x9F
	// SUB B
	instructions_[0x90] = [this]() { Sub(registers_.bc.High()); };
	// SUB C
	instructions_[0x91] = [this]() { Sub(registers_.bc.Low()); };
	// SUB D
	instructions_[0x92] = [this]() { Sub(registers_.de.High()); };
	// SUB E
	instructions_[0x93] = [this]() { Sub(registers_.de.Low()); };
	// SUB H
	instructions_[0x94] = [this]() { Sub(registers_.hl.High()); };
	// SUB L
	instructions_[0x95] = [this]() { Sub(registers_.hl.Low()); };
	// SUB (HL)
	instructions_[0x96] = [this]() { Sub(ReadByte(registers_.hl)); };
	// SUB A
	instructions_[0x97] = [this]() { Sub(registers_.af.High()); }; 
	// SBC A, B
	instructions_[0x98] = [this]() { Sbc(registers_.bc.High()); };
	// SBC A, C
	instructions_[0x99] = [this]() { Sbc(registers_.bc.Low()); };
	// SBC A, D
	instructions_[0x9A] = [this]() { Sbc(registers_.de.High()); };
	// SBC A, E
	instructions_[0x9B] = [this]() { Sbc(registers_.de.Low()); };
	// SBC A, H
	instructions_[0x9C] = [this]() { Sbc(registers_.hl.High()); };
	// SBC A, L
	instructions_[0x9D] = [this]() { Sbc(registers_.hl.Low()); };
	// SBC A, (HL)
	instructions_[0x9E] = [this]() { Sbc(ReadByte(registers_.hl)); };
	// SBC A, A
	instructions_[0x9F] = [this]() { Sbc(registers_.af.High()); };
#pragma endregion

#pragma region Instructions 0xA0 - 0xAF
	// AND B
	instructions_[0xA0] = [this]() { And(registers_.bc.High()); };
	// AND C
	instructions_[0xA1] = [this]() { And(registers_.bc.Low()); };
	// AND D
	instructions_[0xA2] = [this]() { And(registers_.de.High()); };
	// AND E
	instructions_[0xA3] = [this]() { And(registers_.de.Low()); };
	// AND H
	instructions_[0xA4] = [this]() { And(registers_.hl.High()); };
	// AND L
	instructions_[0xA5] = [this]() { And(registers_.hl.Low()); };
	// AND (HL)
	instructions_[0xA6] = [this]() { And(ReadByte(registers_.hl)); };
	// AND A
	instructions_[0xA7] = [this]() { And(registers_.af.High()); };
	// XOR B
	instructions_[0xA8] = [this]() { Xor(registers_.bc.High()); };
	// XOR C
	instructions_[0xA9] = [this]() { Xor(registers_.bc.Low()); };
	// XOR D
	instructions_[0xAA] = [this]() { Xor(registers_.de.High()); };
	// XOR E
	instructions_[0xAB] = [this]() { Xor(registers_.de.Low()); };
	// XOR H
	instructions_[0xAC] = [this]() { Xor(registers_.hl.High()); };
	// XOR L
	instructions_[0xAD] = [this]() { Xor(registers_.hl.Low()); };
	// XOR (HL)
	instructions_[0xAE] = [this]() { Xor(ReadByte(registers_.hl)); };
	// XOR A
	instructions_[0xAF] = [this]() { Xor(registers_.af.High()); };
#pragma endregion

#pragma region Instructions 0xB0 - 0xBF
	// OR B
	instructions_[0xB0] = [this]() { Or(registers_.bc.High()); };
	// OR C
	instructions_[0xB1] = [this]() { Or(registers_.bc.Low()); };
	// OR D
	instructions_[0xB2] = [this]() { Or(registers_.de.High()); };
	// OR E
	instructions_[0xB3] = [this]() { Or(registers_.de.Low()); };
	// OR H
	instructions_[0xB4] = [this]() { Or(registers_.hl.High()); };
	// OR L
	instructions_[0xB5] = [this]() { Or(registers_.hl.Low()); };
	// OR (HL)
	instructions_[0xB6] = [this]() { Or(ReadByte(registers_.hl)); };
	// OR A
	instructions_[0xB7] = [this]() { Or(registers_.af.High()); };
	// CP B
	instructions_[0xB8] = [this]() { Compare(registers_.bc.High()); };
	// CP C
	instructions_[0xB9] = [this]() { Compare(registers_.bc.Low()); };
	// CP D
	instructions_[0xBA] = [this]() { Compare(registers_.de.High()); };
	// CP E
	instructions_[0xBB] = [this]() { Compare(registers_.de.Low()); };
	// CP H
	instructions_[0xBC] = [this]() { Compare(registers_.hl.High()); };
	// CP L
	instructions_[0xBD] = [this]() { Compare(registers_.hl.Low()); };
	// CP (HL)
	instructions_[0xBE] = [this]() { Compare(ReadByte(registers_.hl)); };
	// CP A
	instructions_[0xBF] = [this]() { Compare(registers_.af.High()); };
#pragma endregion

#pragma region Instructions 0xC0 - 0xCF
	// RET NZ
	instructions_[0xC0] = [this]() { NotifyMachineCycleLapse(); if (!IsFlagSet(Flags::Z)) { Return(); } };
	// POP BC
	instructions_[0xC1] = [this]() { registers_.bc = PopWordFromStack(); };
	// JP NZ, a16
	instructions_[0xC2] = [this]() {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::Z))
		{
			registers_.pc = address;
			NotifyMachineCycleLapse();
		}
	};
	// JP a16
	instructions_[0xC3] = [this]() { registers_.pc = FetchWord(); NotifyMachineCycleLapse(); };
	// CALL NZ, a16
	instructions_[0xC4] = [this]() { const auto address = FetchWord(); if (!IsFlagSet(Flags::Z)) { Call(address); } };
	// PUSH BC
	instructions_[0xC5] = [this]() { PushWordToStack(registers_.bc); };
	// ADD A, d8
	instructions_[0xC6] = [this]() { Add(FetchByte()); };
	// RST 00H
	instructions_[0xC7] = [this]() { Call(0x0000); };
	// RET Z
	instructions_[0xC8] = [this]() { NotifyMachineCycleLapse(); if (IsFlagSet(Flags::Z)) { Return(); } };
	// RET
	instructions_[0xC9] = [this]() { Return(); };
	// JP Z, a16
	instructions_[0xCA] = [this]() {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::Z))
		{
			registers_.pc = address;
			NotifyMachineCycleLapse();
		}
	}; 
	// PREFIX CB
	instructions_[0xCB] = [this]() { cb_instructions_[FetchByte()](); };
	// CALL Z, a16
	instructions_[0xCC] = [this]() { const auto address = FetchWord(); if (IsFlagSet(Flags::Z)) { Call(address); } };
	// CALL a16
	instructions_[0xCD] = [this]() { Call(FetchWord()); };
	// ADC A, d8
	instructions_[0xCE] = [this]() { Adc(FetchByte()); };
	// RST 08H
	instructions_[0xCF] = [this]() { Call(0x0008); };
#pragma endregion

#pragma region Instructions 0xD0 - 0xDF
	// RET NC
	instructions_[0xD0] = [this]() {
		NotifyMachineCycleLapse();
		if (!IsFlagSet(Flags::C))
		{
			Return();
		}
	};
	// POP DE
	instructions_[0xD1] = [this]() { registers_.de = PopWordFromStack(); }; 
	// JP NC, a16
	instructions_[0xD2] = [this]() {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::C))
		{
			registers_.pc = address;
			NotifyMachineCycleLapse();
		}
	};
	// CALL NC, a16
	instructions_[0xD4] = [this]() {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::C))
		{
			Call(address);
		}
	};
	// PUSH DE
	instructions_[0xD5] = [this]() { PushWordToStack(registers_.de); };
	// SUB A, d8
	instructions_[0xD6] = [this]() { Sub(FetchByte()); };
	// RST 10H
	instructions_[0xD7] = [this]() { Call(0x0010); };
	// RET C
	instructions_[0xD8] = [this]() {
		NotifyMachineCycleLapse();
		if (IsFlagSet(Flags::C))
		{
			Return();
		}
	};
	// RETI
	instructions_[0xD9] = [this]() { Return(); interrupt_master_enable_ = true; };
	// JP C, a16
	instructions_[0xDA] = [this]() {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::C))
		{
			registers_.pc = address;
			NotifyMachineCycleLapse();
		}
	};
	// CALL C, a16
	instructions_[0xDC] = [this]() {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::C))
		{
			Call(address);
		}
	};
	// SBC A, d8
	instructions_[0xDE] = [this]() { Sbc(FetchByte()); };
	// RST 18H
	instructions_[0xDF] = [this]() { Call(0x0018); };
#pragma endregion

#pragma region Instructions 0xE0 - 0xEF
	// LDH (a8), A
	instructions_[0xE0] = [this]() { WriteByte(Memory::IO + FetchByte(), registers_.af.High()); };
	// POP HL
	instructions_[0xE1] = [this]() { registers_.hl = PopWordFromStack(); };
	// LD (C), A
	instructions_[0xE2] = [this]() { WriteByte(Memory::IO + registers_.bc.Low(), registers_.af.High()); };
	// PUSH HL
	instructions_[0xE5] = [this]() { PushWordToStack(registers_.hl); };
	// AND d8
	instructions_[0xE6] = [this]() { And(FetchByte()); };
	// RST 20H
	instructions_[0xE7] = [this]() { Call(0x0020); };
	// ADD SP, r8
	instructions_[0xE8] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		ClearFlag(Flags::All);
		// Flags are calculated on the lower byte of SP, adding an unsigned displacement
		if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
		if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
		registers_.sp += displacement;
		NotifyMachineCycleLapse();
		NotifyMachineCycleLapse();
	};

	// JP (HL)
	instructions_[0xE9] = [this]() { registers_.pc = registers_.hl; };
	// LD (a16), A
	instructions_[0xEA] = [this]() { WriteByte(FetchWord(), registers_.af.High()); };
	// XOR d8
	instructions_[0xEE] = [this]() { Xor(FetchByte()); };
	// RST 28H
	instructions_[0xEF] = [this]() { Call(0x0028); };
#pragma endregion

#pragma region Instructions 0xF0 - 0xFF
	// LDH A, (a8)
	instructions_[0xF0] = [this]() { registers_.af.High() = ReadByte(Memory::IO + FetchByte()); };
	// POP AF
	instructions_[0xF1] = [this]() { registers_.af = PopWordFromStack() & 0xFFF0; };
	// LD A, (C)
	instructions_[0xF2] = [this]() { registers_.af.High() = ReadByte(Memory::IO + registers_.bc.Low()); };
	// DI
	instructions_[0xF3] = [this]() { interrupt_master_enable_ = false; };
	// PUSH AF
	instructions_[0xF5] = [this]() { PushWordToStack(registers_.af); };
	// OR d8
	instructions_[0xF6] = [this]() { Or(FetchByte()); };
	// RST 30H
	instructions_[0xF7] = [this]() { Call(0x0030); };
	// LD HL, SP+r8
	instructions_[0xF8] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());
		ClearFlag(Flags::All);
		// Flags are calculated on the lower byte of SP, adding an unsigned displacement
		if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
		if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
		registers_.hl = registers_.sp + displacement;
		NotifyMachineCycleLapse();
	};
	// LD SP, HL
	instructions_[0xF9] = [this]() { registers_.sp = registers_.hl; NotifyMachineCycleLapse(); };
	// LD A, (a16)
	instructions_[0xFA] = [this]() { registers_.af.High() = ReadByte(FetchWord()); };
	// EI
	instructions_[0xFB] = [this]() { if (!interrupt_master_enable_) ime_requested_ = true; };
	// CP d8
	instructions_[0xFE] = [this]() { Compare(FetchByte()); };
	// RST 38H
	instructions_[0xFF] = [this]() { Call(0x0038); };
#pragma endregion
}
