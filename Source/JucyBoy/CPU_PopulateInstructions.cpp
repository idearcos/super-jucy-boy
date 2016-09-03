#include "CPU.h"
#include <sstream>
#include <iomanip>
#include "MMU.h"

void CPU::PopulateInstructions()
{
	for (size_t ii = 0; ii < instructions_.size(); ++ii)
	{
		instructions_[ii] = [this, ii]() -> MachineCycles {
			registers_.pc = previous_pc_;
			std::stringstream error;
			error << "Instruction not yet implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << instruction_names_[ii] << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc << ")";
			throw std::runtime_error(error.str());
		};
	}

	// NOP
	instructions_[0x00] = [this]() -> MachineCycles {
		return 1;
	};

	// DEC B
	instructions_[0x05] = [this]() -> MachineCycles {
		ClearFlag(Flags::Z | Flags::H);
		SetFlag(Flags::N);
		const auto value = registers_.bc.GetHigh().Decrement();
		if ((value & 0x0F) == 0x0F) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }
		
		return 1;
	};

	// LD B, d8
	instructions_[0x06] = [this]() -> MachineCycles {
		registers_.bc.GetHigh().Write(FetchByte());
		return 2;
	};

	// DEC C
	instructions_[0x0D] = [this]() -> MachineCycles {
		ClearFlag(Flags::Z | Flags::H);
		SetFlag(Flags::N);
		const auto value = registers_.bc.GetLow().Decrement();
		if ((value & 0x0F) == 0x0F) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }

		return 1;
	};

	// LD C, d8
	instructions_[0x0E] = [this]() -> MachineCycles {
		registers_.bc.GetLow().Write(FetchByte());
		return 2;
	};

	// LD D, d8
	instructions_[0x16] = [this]() -> MachineCycles {
		registers_.de.GetHigh().Write(FetchByte());
		return 2;
	};

	// LD E, d8
	instructions_[0x1E] = [this]() -> MachineCycles {
		registers_.de.GetLow().Write(FetchByte());
		return 2;
	};

	// JR NZ, r8
	instructions_[0x20] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());

		if (IsFlagSet(Flags::Z)) { return 2; }

		registers_.pc += displacement;
		return 3;
	};

	// LD HL, d16
	instructions_[0x21] = [this]() -> MachineCycles {
		registers_.hl.Write(FetchWord());
		return 3;
	};

	// LD (HL+), A
	instructions_[0x32] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl.Read(), registers_.af.GetHigh().Read());
		registers_.hl.Increment();
		return 2;
	};

	// LD H, d8
	instructions_[0x26] = [this]() -> MachineCycles {
		registers_.hl.GetHigh().Write(FetchByte());
		return 2;
	};

	// LD L, d8
	instructions_[0x2E] = [this]() -> MachineCycles {
		registers_.hl.GetLow().Write(FetchByte());
		return 2;
	};

	// LD (HL-), A
	instructions_[0x32] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl.Read(), registers_.af.GetHigh().Read());
		registers_.hl.Decrement();
		return 2;
	};

	// JR C, r8
	instructions_[0x38] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (!IsFlagSet(Flags::C)) { return 2; }
		
		registers_.pc += displacement;
		return 3;
	};

	// LD A, d8
	instructions_[0x3E] = [this]() -> MachineCycles {
		registers_.af.GetHigh().Write(FetchByte());
		return 2;
	};

	// XOR A
	instructions_[0xAF] = [this]() -> MachineCycles {
		const uint8_t value = registers_.af.GetHigh().Read() ^ registers_.af.GetHigh().Read();
		ClearAndSetFlags((value != 0) ? Flags::None : Flags::Z);
		registers_.af.GetHigh().Write(value);
		return 1;
	};

	// JP a16
	instructions_[0xC3] = [this]() -> MachineCycles {
		registers_.pc = FetchWord();
		return 4;
	};

	// LDH (a8), A
	instructions_[0xE0] = [this]() -> MachineCycles {
		mmu_->WriteByte(uint16_t{ 0xFF00 } + FetchByte(), registers_.af.GetHigh().Read());
		return 3;
	};

	// LDH A, (a8)
	instructions_[0xF0] = [this]() -> MachineCycles {
		registers_.af.GetHigh().Write(mmu_->ReadByte(uint16_t{ 0xFF00 } + FetchByte()));
		return 3;
	};

	// DI
	instructions_[0xF3] = [this]() -> MachineCycles {
		//TODO: disable interrupts
		return 1;
	};

	// CP d8
	instructions_[0xFE] = [this]() -> MachineCycles {
		Flags flags_to_set{ Flags::N };

		const auto value = FetchByte();
		const auto acc = registers_.af.GetHigh().Read();

		if ((value & 0x0F) > (acc & 0x0F)) { flags_to_set |= Flags::H; }
		if (value > acc) { flags_to_set |= Flags::C; }
		else if (value == acc) { flags_to_set |= Flags::Z; }

		ClearAndSetFlags(flags_to_set);
		return 2;
	};
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
	//instruction_names_[0xD3] = "";
	instruction_names_[0xD4] = "CALL NC, a16";
	instruction_names_[0xD5] = "PUSH DE";
	instruction_names_[0xD6] = "SUB A, d8";
	instruction_names_[0xD7] = "RST 10H";
	instruction_names_[0xD8] = "RET C";
	instruction_names_[0xD9] = "RETI";
	instruction_names_[0xDA] = "JP C, a16";
	//instruction_names_[0xDB] = "";
	instruction_names_[0xDC] = "CALL C, a16";
	//instruction_names_[0xDD] = "";
	instruction_names_[0xDE] = "SBC A, d8";
	instruction_names_[0xDF] = "RST 18H";

	instruction_names_[0xE0] = "LDH (a8), A";
	instruction_names_[0xE1] = "POP HL";
	instruction_names_[0xE2] = "LD (C), A";
	//instruction_names_[0xE3] = "";
	//instruction_names_[0xE4] = "";
	instruction_names_[0xE5] = "PUSH HL";
	instruction_names_[0xE6] = "AND d8";
	instruction_names_[0xE7] = "RST 20H";
	instruction_names_[0xE8] = "ADD SP, r8";
	instruction_names_[0xE9] = "JP (HL)";
	instruction_names_[0xEA] = "LD (a16), A";
	//instruction_names_[0xEB] = "";
	//instruction_names_[0xEC] = "";
	//instruction_names_[0xED] = "";
	instruction_names_[0xEE] = "XOR d8";
	instruction_names_[0xEF] = "RST 28H";

	instruction_names_[0xF0] = "LDH A, (a8)";
	instruction_names_[0xF1] = "POP AF";
	instruction_names_[0xF2] = "LD A, (C)";
	instruction_names_[0xF3] = "DI";
	//instruction_names_[0xF4] = "";
	instruction_names_[0xF5] = "PUSH AF";
	instruction_names_[0xF6] = "OR d8";
	instruction_names_[0xF7] = "RST 30H";
	instruction_names_[0xF8] = "LD HL SP+r8";
	instruction_names_[0xF9] = "LD SP, HL";
	instruction_names_[0xFA] = "LD A, (a16)";
	instruction_names_[0xFB] = "EI";
	//instruction_names_[0xFC] = "";
	//instruction_names_[0xFD] = "";
	instruction_names_[0xFE] = "CP d8";
	instruction_names_[0xFF] = "RST 38H";
}
