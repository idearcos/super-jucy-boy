#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"

void CPU::PopulateInstructions()
{
	for (size_t ii = 0; ii < instructions_.size(); ++ii)
	{
		instructions_[ii] = [this, ii]() {
			registers_.pc = previous_pc_;
			std::stringstream error;
			error << "Instruction not implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << instruction_names_[ii] << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc << ")";
			throw std::runtime_error(error.str());
		};
	}

#pragma region Instructions 0x00 - 0x0F
	// NOP
	instructions_[0x00] = [this]() {};

	// LD BC, d16
	instructions_[0x01] = [this]() {
		registers_.bc = FetchWord();
	};

	// LD (BC), A
	instructions_[0x02] = [this]() {
		WriteByte(registers_.bc, registers_.af.GetHigh());
	};

	// INC BC
	instructions_[0x03] = [this]() {
		registers_.bc += 1;
		NotifyMachineCycleLapse();
	};

	// INC B
	instructions_[0x04] = [this]() {
		IncrementRegister(registers_.bc.GetHigh());
	};

	// DEC B
	instructions_[0x05] = [this]() {
		DecrementRegister(registers_.bc.GetHigh());
	};

	// LD B, d8
	instructions_[0x06] = [this]() {
		registers_.bc.GetHigh() = FetchByte();
	};

	// RLCA
	instructions_[0x07] = [this]() {
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() << 1) | ((registers_.af.GetHigh() & 0x80) >> 7);
	};

	// LD (a16), SP
	instructions_[0x08] = [this]() {
		auto address = FetchWord();
		WriteByte(address, registers_.sp & 0xFF);
		WriteByte(++address, (registers_.sp >> 8) & 0xFF);
	};

	// ADD HL, BC
	instructions_[0x09] = [this]() {
		AddToHl(registers_.bc);
		NotifyMachineCycleLapse();
	};

	// LD A, (BC)
	instructions_[0x0A] = [this]() {
		registers_.af.GetHigh() = ReadByte(registers_.bc);
	};

	// DEC BC
	instructions_[0x0B] = [this]() {
		--registers_.bc;
		NotifyMachineCycleLapse();
	};

	// INC C
	instructions_[0x0C] = [this]() {
		IncrementRegister(registers_.bc.GetLow());
	};

	// DEC C
	instructions_[0x0D] = [this]() {
		DecrementRegister(registers_.bc.GetLow());
	};

	// LD C, d8
	instructions_[0x0E] = [this]() {
		registers_.bc.GetLow() = FetchByte();
	};

	// RRCA
	instructions_[0x0F] = [this]() {
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() >> 1) | ((registers_.af.GetHigh() & 0x01) << 7);
	};
#pragma endregion

#pragma region Instructions 0x10 - 0x1F
	//TODO STOP 0x10

	// LD DE, d16
	instructions_[0x11] = [this]() {
		registers_.de = FetchWord();
	};

	// LD (DE), A
	instructions_[0x12] = [this]() {
		WriteByte(registers_.de, registers_.af.GetHigh());
	};

	// INC DE
	instructions_[0x13] = [this]() {
		registers_.de += 1;
		NotifyMachineCycleLapse();
	};

	// INC D
	instructions_[0x14] = [this]() {
		IncrementRegister(registers_.de.GetHigh());
	};

	// DEC D
	instructions_[0x15] = [this]() {
		DecrementRegister(registers_.de.GetHigh());
	};

	// LD D, d8
	instructions_[0x16] = [this]() {
		registers_.de.GetHigh() = FetchByte();
	};

	// RLA
	instructions_[0x17] = [this]() {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() << 1) | carry_value;
	};

	// JR r8
	instructions_[0x18] = [this]() {
		const auto displacement = static_cast<int8_t>(FetchByte());

		registers_.pc += displacement;
		NotifyMachineCycleLapse();
	};

	// ADD HL, DE
	instructions_[0x19] = [this]() {
		AddToHl(registers_.de);
		NotifyMachineCycleLapse();
	};

	// LD A, (DE)
	instructions_[0x1A] = [this]() {
		registers_.af.GetHigh() = ReadByte(registers_.de);
	};

	// DEC DE
	instructions_[0x1B] = [this]() {
		--registers_.de;
		NotifyMachineCycleLapse();
	};

	// INC E
	instructions_[0x1C] = [this]() {
		IncrementRegister(registers_.de.GetLow());
	};

	// DEC E
	instructions_[0x1D] = [this]() {
		DecrementRegister(registers_.de.GetLow());
	};

	// LD E, d8
	instructions_[0x1E] = [this]() {
		registers_.de.GetLow() = FetchByte();
	};

	// RRA
	instructions_[0x1F] = [this]() {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() >>= 1;
		registers_.af.GetHigh() |= (carry_value << 7);
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
	instructions_[0x21] = [this]() {
		registers_.hl = FetchWord();
	};

	// LD (HL+), A
	instructions_[0x22] = [this]() {
		WriteByte(registers_.hl++, registers_.af.GetHigh());
	};

	// INC HL
	instructions_[0x23] = [this]() {
		registers_.hl += 1;
		NotifyMachineCycleLapse();
	};

	// INC H
	instructions_[0x24] = [this]() {
		IncrementRegister(registers_.hl.GetHigh());
	};

	// DEC H
	instructions_[0x25] = [this]() {
		DecrementRegister(registers_.hl.GetHigh());
	};

	// LD H, d8
	instructions_[0x26] = [this]() {
		registers_.hl.GetHigh() = FetchByte();
	};

	// DAA
	instructions_[0x27] = [this]() {
		uint8_t correction_factor{ 0 };
		const auto original_value = registers_.af.GetHigh();

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

		registers_.af.GetHigh() = result;
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
	instructions_[0x29] = [this]() {
		AddToHl(registers_.hl);
		NotifyMachineCycleLapse();
	};

	// LD A, (HL+)
	instructions_[0x2A] = [this]() {
		registers_.af.GetHigh() = ReadByte(registers_.hl++);
	};

	// DEC HL
	instructions_[0x2B] = [this]() {
		--registers_.hl;
		NotifyMachineCycleLapse();
	};

	// INC L
	instructions_[0x2C] = [this]() {
		IncrementRegister(registers_.hl.GetLow());
	};

	// DEC L
	instructions_[0x2D] = [this]() {
		DecrementRegister(registers_.hl.GetLow());
	};

	// LD L, d8
	instructions_[0x2E] = [this]() {
		registers_.hl.GetLow() = FetchByte();
	};

	// CPL
	instructions_[0x2F] = [this]() {
		registers_.af.GetHigh() ^= 0xFF;
		SetFlag(Flags::H | Flags::N);
	};
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
	instructions_[0x31] = [this]() {
		registers_.sp = FetchWord();
	};

	// LD (HL-), A
	instructions_[0x32] = [this]() {
		WriteByte(registers_.hl--, registers_.af.GetHigh());
	};

	// INC SP
	instructions_[0x33] = [this]() {
		registers_.sp += 1;
		NotifyMachineCycleLapse();
	};

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
	instructions_[0x36] = [this]() {
		WriteByte(registers_.hl, FetchByte());
	};

	// SCF
	instructions_[0x37] = [this]() {
		SetFlag(Flags::C);
		ClearFlag(Flags::H | Flags::N);
	};

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
	instructions_[0x39] = [this]() {
		AddToHl(registers_.sp);
		NotifyMachineCycleLapse();
	};

	// LD A, (HL-)
	instructions_[0x3A] = [this]() {
		registers_.af.GetHigh() = ReadByte(registers_.hl--);
	};

	// DEC SP
	instructions_[0x3B] = [this]() {
		--registers_.sp;
		NotifyMachineCycleLapse();
	};

	// INC A
	instructions_[0x3C] = [this]() {
		IncrementRegister(registers_.af.GetHigh());
	};

	// DEC A
	instructions_[0x3D] = [this]() {
		DecrementRegister(registers_.af.GetHigh());
	};

	// LD A, d8
	instructions_[0x3E] = [this]() {
		registers_.af.GetHigh() = FetchByte();
	};

	// CCF
	instructions_[0x3F] = [this]() {
		ToggleFlag(Flags::C);
		ClearFlag(Flags::H | Flags::N);
	};
#pragma endregion

#pragma region Instructions 0x40 - 0x4F
	// LD B, B
	instructions_[0x40] = [this]() {};

	// LD B, C
	instructions_[0x41] = [this]() {
		registers_.bc.GetHigh() = registers_.bc.GetLow();
	};

	// LD B, D
	instructions_[0x42] = [this]() {
		registers_.bc.GetHigh() = registers_.de.GetHigh();
	};

	// LD B, E
	instructions_[0x43] = [this]() {
		registers_.bc.GetHigh() = registers_.de.GetLow();
	};

	// LD B, H
	instructions_[0x44] = [this]() {
		registers_.bc.GetHigh() = registers_.hl.GetHigh();
	};

	// LD B, L
	instructions_[0x45] = [this]() {
		registers_.bc.GetHigh() = registers_.hl.GetLow();
	};

	// LD B, (HL)
	instructions_[0x46] = [this]() {
		registers_.bc.GetHigh() = ReadByte(registers_.hl);
	};

	// LD B, A
	instructions_[0x47] = [this]() {
		registers_.bc.GetHigh() = registers_.af.GetHigh();
	};

	// LD C, B
	instructions_[0x48] = [this]() {
		registers_.bc.GetLow() = registers_.bc.GetHigh();
	};

	// LD C, C
	instructions_[0x49] = [this]() {};

	// LD C, D
	instructions_[0x4A] = [this]() {
		registers_.bc.GetLow() = registers_.de.GetHigh();
	};

	// LD C, E
	instructions_[0x4B] = [this]() {
		registers_.bc.GetLow() = registers_.de.GetLow();
	};

	// LD C, H
	instructions_[0x4C] = [this]() {
		registers_.bc.GetLow() = registers_.hl.GetHigh();
	};

	// LD C, L
	instructions_[0x4D] = [this]() {
		registers_.bc.GetLow() = registers_.hl.GetLow();
	};

	// LD C, (HL)
	instructions_[0x4E] = [this]() {
		registers_.bc.GetLow() = ReadByte(registers_.hl);
	};

	// LD C, A
	instructions_[0x4F] = [this]() {
		registers_.bc.GetLow() = registers_.af.GetHigh();
	};
#pragma endregion

#pragma region Instructions 0x50 - 0x5F
	// LD D, B
	instructions_[0x50] = [this]() {
		registers_.de.GetHigh() = registers_.bc.GetHigh();
	};

	// LD D, C
	instructions_[0x51] = [this]() {
		registers_.de.GetHigh() = registers_.bc.GetLow();
	};

	// LD D, D
	instructions_[0x52] = [this]() {};

	// LD D, E
	instructions_[0x53] = [this]() {
		registers_.de.GetHigh() = registers_.de.GetLow();
	};

	// LD D, H
	instructions_[0x54] = [this]() {
		registers_.de.GetHigh() = registers_.hl.GetHigh();
	};

	// LD D, L
	instructions_[0x55] = [this]() {
		registers_.de.GetHigh() = registers_.hl.GetLow();
	};

	// LD D, (HL)
	instructions_[0x56] = [this]() {
		registers_.de.GetHigh() = ReadByte(registers_.hl);
	};

	// LD D, A
	instructions_[0x57] = [this]() {
		registers_.de.GetHigh() = registers_.af.GetHigh();
	};

	// LD E, B
	instructions_[0x58] = [this]() {
		registers_.de.GetLow() = registers_.bc.GetHigh();
	};

	// LD E, C
	instructions_[0x59] = [this]() {
		registers_.de.GetLow() = registers_.bc.GetLow();
	};

	// LD E, D
	instructions_[0x5A] = [this]() {
		registers_.de.GetLow() = registers_.de.GetHigh();
	};

	// LD E, E
	instructions_[0x5B] = [this]() {};

	// LD E, H
	instructions_[0x5C] = [this]() {
		registers_.de.GetLow() = registers_.hl.GetHigh();
	};

	// LD E, L
	instructions_[0x5D] = [this]() {
		registers_.de.GetLow() = registers_.hl.GetLow();
	};

	// LD E, (HL)
	instructions_[0x5E] = [this]() {
		registers_.de.GetLow() = ReadByte(registers_.hl);
	};

	// LD E, A
	instructions_[0x5F] = [this]() {
		registers_.de.GetLow() = registers_.af.GetHigh();
	};
#pragma endregion

#pragma region Instructions 0x60 - 0x6F
	// LD H, B
	instructions_[0x60] = [this]() {
		registers_.hl.GetHigh() = registers_.bc.GetHigh();
	};

	// LD H, C
	instructions_[0x61] = [this]() {
		registers_.hl.GetHigh() = registers_.bc.GetLow();
	};

	// LD H, D
	instructions_[0x62] = [this]() {
		registers_.hl.GetHigh() = registers_.de.GetHigh();
	};

	// LD H, E
	instructions_[0x63] = [this]() {
		registers_.hl.GetHigh() = registers_.de.GetLow();
	};

	// LD H, H
	instructions_[0x64] = [this]() {};

	// LD H, L
	instructions_[0x65] = [this]() {
		registers_.hl.GetHigh() = registers_.hl.GetLow();
	};

	// LD H, (HL)
	instructions_[0x66] = [this]() {
		registers_.hl.GetHigh() = ReadByte(registers_.hl);
	};

	// LD H, A
	instructions_[0x67] = [this]() {
		registers_.hl.GetHigh() = registers_.af.GetHigh();
	};

	// LD L, B
	instructions_[0x68] = [this]() {
		registers_.hl.GetLow() = registers_.bc.GetHigh();
	};

	// LD L, C
	instructions_[0x69] = [this]() {
		registers_.hl.GetLow() = registers_.bc.GetLow();
	};

	// LD L, D
	instructions_[0x6A] = [this]() {
		registers_.hl.GetLow() = registers_.de.GetHigh();
	};

	// LD L, E
	instructions_[0x6B] = [this]() {
		registers_.hl.GetLow() = registers_.de.GetLow();
	};

	// LD L, H
	instructions_[0x6C] = [this]() {
		registers_.hl.GetLow() = registers_.hl.GetHigh();
	};

	// LD L, L
	instructions_[0x6D] = [this]() {};

	// LD L, (HL)
	instructions_[0x6E] = [this]() {
		registers_.hl.GetLow() = ReadByte(registers_.hl);
	};

	// LD L, A
	instructions_[0x6F] = [this]() {
		registers_.hl.GetLow() = registers_.af.GetHigh();
	};
#pragma endregion

#pragma region Instructions 0x70 - 0x7F
	// LD (HL), B
	instructions_[0x70] = [this]() {
		WriteByte(registers_.hl, registers_.bc.GetHigh());
	};

	// LD (HL), C
	instructions_[0x71] = [this]() {
		WriteByte(registers_.hl, registers_.bc.GetLow());
	};

	// LD (HL), D
	instructions_[0x72] = [this]() {
		WriteByte(registers_.hl, registers_.de.GetHigh());
	};

	// LD (HL), E
	instructions_[0x73] = [this]() {
		WriteByte(registers_.hl, registers_.de.GetLow());
	};

	// LD (HL), H
	instructions_[0x74] = [this]() {
		WriteByte(registers_.hl, registers_.hl.GetHigh());
	};

	// LD (HL), L
	instructions_[0x75] = [this]() {
		WriteByte(registers_.hl, registers_.hl.GetLow());
	};

	// HALT
	instructions_[0x76] = [this]() {
		current_state_ = State::Halted;
	};

	// LD (HL), A
	instructions_[0x77] = [this]() {
		WriteByte(registers_.hl, registers_.af.GetHigh());
	};

	// LD A, B
	instructions_[0x78] = [this]() {
		registers_.af.GetHigh() = registers_.bc.GetHigh();
	};

	// LD A, C
	instructions_[0x79] = [this]() {
		registers_.af.GetHigh() = registers_.bc.GetLow();
	};

	// LD A, D
	instructions_[0x7A] = [this]() {
		registers_.af.GetHigh() = registers_.de.GetHigh();
	};

	// LD A, E
	instructions_[0x7B] = [this]() {
		registers_.af.GetHigh() = registers_.de.GetLow();
	};

	// LD A, H
	instructions_[0x7C] = [this]() {
		registers_.af.GetHigh() = registers_.hl.GetHigh();
	};

	// LD A, L
	instructions_[0x7D] = [this]() {
		registers_.af.GetHigh() = registers_.hl.GetLow();
	};

	// LD A, (HL)
	instructions_[0x7E] = [this]() {
		registers_.af.GetHigh() = ReadByte(registers_.hl);
	};

	// LD A, A
	instructions_[0x7F] = [this]() {};
#pragma endregion

#pragma region Instructions 0x80 - 0x8F
	// ADD A, B
	instructions_[0x80] = [this]() {
		Add(registers_.bc.GetHigh());
	};

	// ADD A, C
	instructions_[0x81] = [this]() {
		Add(registers_.bc.GetLow());
	};

	// ADD A, D
	instructions_[0x82] = [this]() {
		Add(registers_.de.GetHigh());
	};

	// ADD A, E
	instructions_[0x83] = [this]() {
		Add(registers_.de.GetLow());
	};

	// ADD A, H
	instructions_[0x84] = [this]() {
		Add(registers_.hl.GetHigh());
	};

	// ADD A, L
	instructions_[0x85] = [this]() {
		Add(registers_.hl.GetLow());
	};

	// ADD (HL)
	instructions_[0x86] = [this]() {
		Add(ReadByte(registers_.hl));
	};

	// ADD A, A
	instructions_[0x87] = [this]() {
		Add(registers_.af.GetHigh());
	};

	// ADC A, B
	instructions_[0x88] = [this]() {
		Adc(registers_.bc.GetHigh());
	};

	// ADC A, C
	instructions_[0x89] = [this]() {
		Adc(registers_.bc.GetLow());
	};

	// ADC A, D
	instructions_[0x8A] = [this]() {
		Adc(registers_.de.GetHigh());
	};

	// ADC A, E
	instructions_[0x8B] = [this]() {
		Adc(registers_.de.GetLow());
	};

	// ADC A, H
	instructions_[0x8C] = [this]() {
		Adc(registers_.hl.GetHigh());
	};

	// ADC A, L
	instructions_[0x8D] = [this]() {
		Adc(registers_.hl.GetLow());
	};

	// ADC A, (HL)
	instructions_[0x8E] = [this]() {
		Adc(ReadByte(registers_.hl));
	};

	// ADC A, A
	instructions_[0x8F] = [this]() {
		Adc(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region Instructions 0x90 - 0x9F
	// SUB B
	instructions_[0x90] = [this]() {
		Sub(registers_.bc.GetHigh());
	};

	// SUB C
	instructions_[0x91] = [this]() {
		Sub(registers_.bc.GetLow());
	};

	// SUB D
	instructions_[0x92] = [this]() {
		Sub(registers_.de.GetHigh());
	};

	// SUB E
	instructions_[0x93] = [this]() {
		Sub(registers_.de.GetLow());
	};

	// SUB H
	instructions_[0x94] = [this]() {
		Sub(registers_.hl.GetHigh());
	};

	// SUB L
	instructions_[0x95] = [this]() {
		Sub(registers_.hl.GetLow());
	};

	// SUB (HL)
	instructions_[0x96] = [this]() {
		Sub(ReadByte(registers_.hl));
	};

	// SUB A
	instructions_[0x97] = [this]() {
		Sub(registers_.af.GetHigh());
	};

	// SBC A, B
	instructions_[0x98] = [this]() {
		Sbc(registers_.bc.GetHigh());
	};

	// SBC A, C
	instructions_[0x99] = [this]() {
		Sbc(registers_.bc.GetLow());
	};

	// SBC A, D
	instructions_[0x9A] = [this]() {
		Sbc(registers_.de.GetHigh());
	};

	// SBC A, E
	instructions_[0x9B] = [this]() {
		Sbc(registers_.de.GetLow());
	};

	// SBC A, H
	instructions_[0x9C] = [this]() {
		Sbc(registers_.hl.GetHigh());
	};

	// SBC A, L
	instructions_[0x9D] = [this]() {
		Sbc(registers_.hl.GetLow());
	};

	// SBC A, (HL)
	instructions_[0x9E] = [this]() {
		Sbc(ReadByte(registers_.hl));
	};

	// SBC A, A
	instructions_[0x9F] = [this]() {
		Sbc(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region Instructions 0xA0 - 0xAF
	// AND B
	instructions_[0xA0] = [this]() {
		And(registers_.bc.GetHigh());
	};

	// AND C
	instructions_[0xA1] = [this]() {
		And(registers_.bc.GetLow());
	};

	// AND D
	instructions_[0xA2] = [this]() {
		And(registers_.de.GetHigh());
	};

	// AND E
	instructions_[0xA3] = [this]() {
		And(registers_.de.GetLow());
	};

	// AND H
	instructions_[0xA4] = [this]() {
		And(registers_.hl.GetHigh());
	};

	// AND L
	instructions_[0xA5] = [this]() {
		And(registers_.hl.GetLow());
	};

	// AND (HL)
	instructions_[0xA6] = [this]() {
		And(ReadByte(registers_.hl));
	};

	// AND A
	instructions_[0xA7] = [this]() {
		And(registers_.af.GetHigh());
	};

	// XOR B
	instructions_[0xA8] = [this]() {
		Xor(registers_.bc.GetHigh());
	};

	// XOR C
	instructions_[0xA9] = [this]() {
		Xor(registers_.bc.GetLow());
	};

	// XOR D
	instructions_[0xAA] = [this]() {
		Xor(registers_.de.GetHigh());
	};

	// XOR E
	instructions_[0xAB] = [this]() {
		Xor(registers_.de.GetLow());
	};

	// XOR H
	instructions_[0xAC] = [this]() {
		Xor(registers_.hl.GetHigh());
	};

	// XOR L
	instructions_[0xAD] = [this]() {
		Xor(registers_.hl.GetLow());
	};

	// XOR (HL)
	instructions_[0xAE] = [this]() {
		Xor(ReadByte(registers_.hl));
	};

	// XOR A
	instructions_[0xAF] = [this]() {
		Xor(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region Instructions 0xB0 - 0xBF
	// OR B
	instructions_[0xB0] = [this]() {
		Or(registers_.bc.GetHigh());
	};

	// OR C
	instructions_[0xB1] = [this]() {
		Or(registers_.bc.GetLow());
	};

	// OR D
	instructions_[0xB2] = [this]() {
		Or(registers_.de.GetHigh());
	};

	// OR E
	instructions_[0xB3] = [this]() {
		Or(registers_.de.GetLow());
	};

	// OR H
	instructions_[0xB4] = [this]() {
		Or(registers_.hl.GetHigh());
	};

	// OR L
	instructions_[0xB5] = [this]() {
		Or(registers_.hl.GetLow());
	};

	// OR (HL)
	instructions_[0xB6] = [this]() {
		Or(ReadByte(registers_.hl));
	};

	// OR A
	instructions_[0xB7] = [this]() {
		Or(registers_.af.GetHigh());
	};

	// CP B
	instructions_[0xB8] = [this]() {
		Compare(registers_.bc.GetHigh());
	};

	// CP C
	instructions_[0xB9] = [this]() {
		Compare(registers_.bc.GetLow());
	};

	// CP D
	instructions_[0xBA] = [this]() {
		Compare(registers_.de.GetHigh());
	};

	// CP E
	instructions_[0xBB] = [this]() {
		Compare(registers_.de.GetLow());
	};

	// CP H
	instructions_[0xBC] = [this]() {
		Compare(registers_.hl.GetHigh());
	};

	// CP L
	instructions_[0xBD] = [this]() {
		Compare(registers_.hl.GetLow());
	};

	// CP (HL)
	instructions_[0xBE] = [this]() {
		Compare(ReadByte(registers_.hl));
	};

	// CP A
	instructions_[0xBF] = [this]() {
		Compare(registers_.af.GetHigh());
	};
#pragma endregion

#pragma region Instructions 0xC0 - 0xCF
	// RET NZ
	instructions_[0xC0] = [this]() {
		NotifyMachineCycleLapse();
		if (!IsFlagSet(Flags::Z))
		{
			Return();
		}
	};

	// POP BC
	instructions_[0xC1] = [this]() {
		registers_.bc = PopWordFromStack();
	};

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
	instructions_[0xC3] = [this]() {
		registers_.pc = FetchWord();
		NotifyMachineCycleLapse();
	};

	// CALL NZ, a16
	instructions_[0xC4] = [this]() {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::Z))
		{
			Call(address);
		}
	};

	// PUSH BC
	instructions_[0xC5] = [this]() {
		PushWordToStack(registers_.bc);
	};

	// ADD A, d8
	instructions_[0xC6] = [this]() {
		Add(FetchByte());
	};

	// RST 00H
	instructions_[0xC7] = [this]() {
		Call(0x0000);
	};

	// RET Z
	instructions_[0xC8] = [this]() {
		NotifyMachineCycleLapse();
		if (IsFlagSet(Flags::Z))
		{
			Return();
		}
	};

	// RET
	instructions_[0xC9] = [this]() {
		Return();
	};

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
	instructions_[0xCB] = [this]() {
		cb_instructions_[FetchByte()]();
	};

	// CALL Z, a16
	instructions_[0xCC] = [this]() {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::Z))
		{
			Call(address);
		}
	};

	// CALL a16
	instructions_[0xCD] = [this]() {
		Call(FetchWord());
	};

	// ADC A, d8
	instructions_[0xCE] = [this]() {
		Adc(FetchByte());
	};

	// RST 08H
	instructions_[0xCF] = [this]() {
		Call(0x0008);
	};
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
	instructions_[0xD1] = [this]() {
		registers_.de = PopWordFromStack();
	};

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
	instructions_[0xD5] = [this]() {
		PushWordToStack(registers_.de);
	};

	// SUB A, d8
	instructions_[0xD6] = [this]() {
		Sub(FetchByte());
	};

	// RST 10H
	instructions_[0xD7] = [this]() {
		Call(0x0010);
	};

	// RET C
	instructions_[0xD8] = [this]() {
		NotifyMachineCycleLapse();
		if (IsFlagSet(Flags::C))
		{
			Return();
		}
	};

	// RETI
	instructions_[0xD9] = [this]() {
		Return();
		interrupt_master_enable_ = true;
	};

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
	instructions_[0xDE] = [this]() {
		Sbc(FetchByte());
	};

	// RST 18H
	instructions_[0xDF] = [this]() {
		Call(0x0018);
	};
#pragma endregion

#pragma region Instructions 0xE0 - 0xEF
	// LDH (a8), A
	instructions_[0xE0] = [this]() {
		WriteByte(Memory::io_region_start_ + FetchByte(), registers_.af.GetHigh());
	};

	// POP HL
	instructions_[0xE1] = [this]() {
		registers_.hl = PopWordFromStack();
	};

	// LD (C), A
	instructions_[0xE2] = [this]() {
		WriteByte(Memory::io_region_start_ + registers_.bc.GetLow(), registers_.af.GetHigh());
	};

	// PUSH HL
	instructions_[0xE5] = [this]() {
		PushWordToStack(registers_.hl);
	};

	// AND d8
	instructions_[0xE6] = [this]() {
		And(FetchByte());
	};

	// RST 20H
	instructions_[0xE7] = [this]() {
		Call(0x0020);
	};

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
	instructions_[0xE9] = [this]() {
		registers_.pc = registers_.hl;
	};

	// LD (a16), A
	instructions_[0xEA] = [this]() {
		WriteByte(FetchWord(), registers_.af.GetHigh());
	};

	// XOR d8
	instructions_[0xEE] = [this]() {
		Xor(FetchByte());
	};

	// RST 28H
	instructions_[0xEF] = [this]() {
		Call(0x0028);
	};
#pragma endregion

#pragma region Instructions 0xF0 - 0xFF
	// LDH A, (a8)
	instructions_[0xF0] = [this]() {
		registers_.af.GetHigh() = ReadByte(Memory::io_region_start_ + FetchByte());
	};

	// POP AF
	instructions_[0xF1] = [this]() {
		registers_.af = PopWordFromStack() & 0xFFF0;
	};

	// LD A, (C)
	instructions_[0xF2] = [this]() {
		registers_.af.GetHigh() = ReadByte(Memory::io_region_start_ + registers_.bc.GetLow());
	};

	// DI
	instructions_[0xF3] = [this]() {
		interrupt_master_enable_ = false;
	};

	// PUSH AF
	instructions_[0xF5] = [this]() {
		PushWordToStack(registers_.af);
	};

	// OR d8
	instructions_[0xF6] = [this]() {
		Or(FetchByte());
	};

	// RST 30H
	instructions_[0xF7] = [this]() {
		Call(0x0030);
	};

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
	instructions_[0xF9] = [this]() {
		registers_.sp = registers_.hl;
		NotifyMachineCycleLapse();
	};

	// LD A, (a16)
	instructions_[0xFA] = [this]() {
		registers_.af.GetHigh() = ReadByte(FetchWord());
	};

	// EI
	instructions_[0xFB] = [this]() {
		if (!interrupt_master_enable_) ime_requested_ = true;
	};

	// CP d8
	instructions_[0xFE] = [this]() {
		Compare(FetchByte());
	};

	// RST 38H
	instructions_[0xFF] = [this]() {
		Call(0x0038);
	};
#pragma endregion
}

void CPU::PopulateInstructionNames()
{
	instruction_names_[0x00] = "NOP";
	instruction_names_[0x01] = "LD BC, d16";
	instruction_names_[0x02] = "LD (BC), A";
	instruction_names_[0x03] = "INC BC";
	instruction_names_[0x04] = "INC B";
	instruction_names_[0x05] = "DEC B";
	instruction_names_[0x06] = "LD B, d8";
	instruction_names_[0x07] = "RLCA";
	instruction_names_[0x08] = "LD (a16), SP";
	instruction_names_[0x09] = "ADD HL, BC";
	instruction_names_[0x0A] = "LD A, (BC)";
	instruction_names_[0x0B] = "DEC BC";
	instruction_names_[0x0C] = "INC C";
	instruction_names_[0x0D] = "DEC C";
	instruction_names_[0x0E] = "LD C, d8";
	instruction_names_[0x0F] = "RRCA";

	instruction_names_[0x10] = "STOP";
	instruction_names_[0x11] = "LD DE, d16";
	instruction_names_[0x12] = "LD (DE), A";
	instruction_names_[0x13] = "INC DE";
	instruction_names_[0x14] = "INC D";
	instruction_names_[0x15] = "DEC D";
	instruction_names_[0x16] = "LD D, d8";
	instruction_names_[0x17] = "RLA";
	instruction_names_[0x18] = "JR r8";
	instruction_names_[0x19] = "ADD HL, DE";
	instruction_names_[0x1A] = "LD A, (DE)";
	instruction_names_[0x1B] = "DEC DE";
	instruction_names_[0x1C] = "INC E";
	instruction_names_[0x1D] = "DEC E";
	instruction_names_[0x1E] = "LD E, d8";
	instruction_names_[0x1F] = "RRA";

	instruction_names_[0x20] = "JR NZ, r8";
	instruction_names_[0x21] = "LD HL, d16";
	instruction_names_[0x22] = "LD (HL+), A";
	instruction_names_[0x23] = "INC HL";
	instruction_names_[0x24] = "INC H";
	instruction_names_[0x25] = "DEC H";
	instruction_names_[0x26] = "LD H, d8";
	instruction_names_[0x27] = "DAA";
	instruction_names_[0x28] = "JR Z, r8";
	instruction_names_[0x29] = "ADD HL, HL";
	instruction_names_[0x2A] = "LD A, (HL+)";
	instruction_names_[0x2B] = "DEC HL";
	instruction_names_[0x2C] = "INC L";
	instruction_names_[0x2D] = "DEC L";
	instruction_names_[0x2E] = "LD L, d8";
	instruction_names_[0x2F] = "CPL";

	instruction_names_[0x30] = "JR NC, r8";
	instruction_names_[0x31] = "LD SP, d16";
	instruction_names_[0x32] = "LD (HL-), A";
	instruction_names_[0x33] = "INC SP";
	instruction_names_[0x34] = "INC (HL)";
	instruction_names_[0x35] = "DEC (HL)";
	instruction_names_[0x36] = "LD (HL), d8";
	instruction_names_[0x37] = "SCF";
	instruction_names_[0x38] = "JR C, r8";
	instruction_names_[0x39] = "ADD HL, SP";
	instruction_names_[0x3A] = "LD A, (HL-)";
	instruction_names_[0x3B] = "DEC SP";
	instruction_names_[0x3C] = "INC A";
	instruction_names_[0x3D] = "DEC A";
	instruction_names_[0x3E] = "LD A, d8";
	instruction_names_[0x3F] = "CCF";

	instruction_names_[0x40] = "LD B, B";
	instruction_names_[0x41] = "LD B, C";
	instruction_names_[0x42] = "LD B, D";
	instruction_names_[0x43] = "LD B, E";
	instruction_names_[0x44] = "LD B, H";
	instruction_names_[0x45] = "LD B, L";
	instruction_names_[0x46] = "LD B, (HL)";
	instruction_names_[0x47] = "LD B, A";
	instruction_names_[0x48] = "LD C, B";
	instruction_names_[0x49] = "LD C, C";
	instruction_names_[0x4A] = "LD C, D";
	instruction_names_[0x4B] = "LD C, E";
	instruction_names_[0x4C] = "LD C, H";
	instruction_names_[0x4D] = "LD C, L";
	instruction_names_[0x4E] = "LD C, (HL)";
	instruction_names_[0x4F] = "LD C, A";

	instruction_names_[0x50] = "LD D, B";
	instruction_names_[0x51] = "LD D, C";
	instruction_names_[0x52] = "LD D, D";
	instruction_names_[0x53] = "LD D, E";
	instruction_names_[0x54] = "LD D, H";
	instruction_names_[0x55] = "LD D, L";
	instruction_names_[0x56] = "LD D, (HL)";
	instruction_names_[0x57] = "LD D, A";
	instruction_names_[0x58] = "LD E, B";
	instruction_names_[0x59] = "LD E, C";
	instruction_names_[0x5A] = "LD E, D";
	instruction_names_[0x5B] = "LD E, E";
	instruction_names_[0x5C] = "LD E, H";
	instruction_names_[0x5D] = "LD E, L";
	instruction_names_[0x5E] = "LD E, (HL)";
	instruction_names_[0x5F] = "LD E, A";

	instruction_names_[0x60] = "LD H, B";
	instruction_names_[0x61] = "LD H, C";
	instruction_names_[0x62] = "LD H, D";
	instruction_names_[0x63] = "LD H, E";
	instruction_names_[0x64] = "LD H, H";
	instruction_names_[0x65] = "LD H, L";
	instruction_names_[0x66] = "LD H, (HL)";
	instruction_names_[0x67] = "LD H, A";
	instruction_names_[0x68] = "LD L, B";
	instruction_names_[0x69] = "LD L, C";
	instruction_names_[0x6A] = "LD L, D";
	instruction_names_[0x6B] = "LD L, E";
	instruction_names_[0x6C] = "LD L, H";
	instruction_names_[0x6D] = "LD L, L";
	instruction_names_[0x6E] = "LD L, (HL)";
	instruction_names_[0x6F] = "LD L, A";

	instruction_names_[0x70] = "LD (HL), B";
	instruction_names_[0x71] = "LD (HL), C";
	instruction_names_[0x72] = "LD (HL), D";
	instruction_names_[0x73] = "LD (HL), E";
	instruction_names_[0x74] = "LD (HL), H";
	instruction_names_[0x75] = "LD (HL), L";
	instruction_names_[0x76] = "HALT";
	instruction_names_[0x77] = "LD (HL), A";
	instruction_names_[0x78] = "LD A, B";
	instruction_names_[0x79] = "LD A, C";
	instruction_names_[0x7A] = "LD A, D";
	instruction_names_[0x7B] = "LD A, E";
	instruction_names_[0x7C] = "LD A, H";
	instruction_names_[0x7D] = "LD A, L";
	instruction_names_[0x7E] = "LD A, (HL)";
	instruction_names_[0x7F] = "LD A, A";

	instruction_names_[0x80] = "ADD A, B";
	instruction_names_[0x81] = "ADD A, C";
	instruction_names_[0x82] = "ADD A, D";
	instruction_names_[0x83] = "ADD A, E";
	instruction_names_[0x84] = "ADD A, H";
	instruction_names_[0x85] = "ADD A, L";
	instruction_names_[0x86] = "ADD A, (HL)";
	instruction_names_[0x87] = "ADD A, A";
	instruction_names_[0x88] = "ADC A, B";
	instruction_names_[0x89] = "ADC A, C";
	instruction_names_[0x8A] = "ADC A, D";
	instruction_names_[0x8B] = "ADC A, E";
	instruction_names_[0x8C] = "ADC A, H";
	instruction_names_[0x8D] = "ADC A, L";
	instruction_names_[0x8E] = "ADC A, (HL)";
	instruction_names_[0x8F] = "ADC A, A";

	instruction_names_[0x90] = "SUB A, B";
	instruction_names_[0x91] = "SUB A, C";
	instruction_names_[0x92] = "SUB A, D";
	instruction_names_[0x93] = "SUB A, E";
	instruction_names_[0x94] = "SUB A, H";
	instruction_names_[0x95] = "SUB A, L";
	instruction_names_[0x96] = "SUB A, (HL)";
	instruction_names_[0x97] = "SUB A, A";
	instruction_names_[0x98] = "SBC A, B";
	instruction_names_[0x99] = "SBC A, C";
	instruction_names_[0x9A] = "SBC A, D";
	instruction_names_[0x9B] = "SBC A, E";
	instruction_names_[0x9C] = "SBC A, H";
	instruction_names_[0x9D] = "SBC A, L";
	instruction_names_[0x9E] = "SBC A, (HL)";
	instruction_names_[0x9F] = "SBC A, A";

	instruction_names_[0xA0] = "AND B";
	instruction_names_[0xA1] = "AND C";
	instruction_names_[0xA2] = "AND D";
	instruction_names_[0xA3] = "AND E";
	instruction_names_[0xA4] = "AND H";
	instruction_names_[0xA5] = "AND L";
	instruction_names_[0xA6] = "AND (HL)";
	instruction_names_[0xA7] = "AND A";
	instruction_names_[0xA8] = "XOR B";
	instruction_names_[0xA9] = "XOR C";
	instruction_names_[0xAA] = "XOR D";
	instruction_names_[0xAB] = "XOR E";
	instruction_names_[0xAC] = "XOR H";
	instruction_names_[0xAD] = "XOR L";
	instruction_names_[0xAE] = "XOR (HL)";
	instruction_names_[0xAF] = "XOR A";

	instruction_names_[0xB0] = "OR B";
	instruction_names_[0xB1] = "OR C";
	instruction_names_[0xB2] = "OR D";
	instruction_names_[0xB3] = "OR E";
	instruction_names_[0xB4] = "OR H";
	instruction_names_[0xB5] = "OR L";
	instruction_names_[0xB6] = "OR (HL)";
	instruction_names_[0xB7] = "OR A";
	instruction_names_[0xB8] = "CP B";
	instruction_names_[0xB9] = "CP C";
	instruction_names_[0xBA] = "CP D";
	instruction_names_[0xBB] = "CP E";
	instruction_names_[0xBC] = "CP H";
	instruction_names_[0xBD] = "CP L";
	instruction_names_[0xBE] = "CP (HL)";
	instruction_names_[0xBF] = "CP A";

	instruction_names_[0xC0] = "RET NZ";
	instruction_names_[0xC1] = "POP BC";
	instruction_names_[0xC2] = "JP NZ, a16";
	instruction_names_[0xC3] = "JP a16";
	instruction_names_[0xC4] = "CALL NZ, a16";
	instruction_names_[0xC5] = "PUSH BC";
	instruction_names_[0xC6] = "ADD A, d8";
	instruction_names_[0xC7] = "RST 00H";
	instruction_names_[0xC8] = "RET Z";
	instruction_names_[0xC9] = "RET";
	instruction_names_[0xCA] = "JP Z, a16";
	instruction_names_[0xCB] = "PREFIX CB";
	instruction_names_[0xCC] = "CALL Z, a16";
	instruction_names_[0xCD] = "CALL a16";
	instruction_names_[0xCE] = "ADC A, d8";
	instruction_names_[0xCF] = "RST 08H";

	instruction_names_[0xD0] = "RET NC";
	instruction_names_[0xD1] = "POP DE";
	instruction_names_[0xD2] = "JP NC, a16";
	instruction_names_[0xD3] = "-";
	instruction_names_[0xD4] = "CALL NC, a16";
	instruction_names_[0xD5] = "PUSH DE";
	instruction_names_[0xD6] = "SUB A, d8";
	instruction_names_[0xD7] = "RST 10H";
	instruction_names_[0xD8] = "RET C";
	instruction_names_[0xD9] = "RETI";
	instruction_names_[0xDA] = "JP C, a16";
	instruction_names_[0xDB] = "-";
	instruction_names_[0xDC] = "CALL C, a16";
	instruction_names_[0xDD] = "-";
	instruction_names_[0xDE] = "SBC A, d8";
	instruction_names_[0xDF] = "RST 18H";

	instruction_names_[0xE0] = "LDH (a8), A";
	instruction_names_[0xE1] = "POP HL";
	instruction_names_[0xE2] = "LD (C), A";
	instruction_names_[0xE3] = "-";
	instruction_names_[0xE4] = "-";
	instruction_names_[0xE5] = "PUSH HL";
	instruction_names_[0xE6] = "AND d8";
	instruction_names_[0xE7] = "RST 20H";
	instruction_names_[0xE8] = "ADD SP, r8";
	instruction_names_[0xE9] = "JP (HL)";
	instruction_names_[0xEA] = "LD (a16), A";
	instruction_names_[0xEB] = "-";
	instruction_names_[0xEC] = "-";
	instruction_names_[0xED] = "-";
	instruction_names_[0xEE] = "XOR d8";
	instruction_names_[0xEF] = "RST 28H";

	instruction_names_[0xF0] = "LDH A, (a8)";
	instruction_names_[0xF1] = "POP AF";
	instruction_names_[0xF2] = "LD A, (C)";
	instruction_names_[0xF3] = "DI";
	instruction_names_[0xF4] = "-";
	instruction_names_[0xF5] = "PUSH AF";
	instruction_names_[0xF6] = "OR d8";
	instruction_names_[0xF7] = "RST 30H";
	instruction_names_[0xF8] = "LD HL, SP+r8";
	instruction_names_[0xF9] = "LD SP, HL";
	instruction_names_[0xFA] = "LD A, (a16)";
	instruction_names_[0xFB] = "EI";
	instruction_names_[0xFC] = "-";
	instruction_names_[0xFD] = "-";
	instruction_names_[0xFE] = "CP d8";
	instruction_names_[0xFF] = "RST 38H";
}
