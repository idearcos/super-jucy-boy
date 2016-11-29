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

#pragma region Instructions 0x00 - 0x0F
	// NOP
	instructions_[0x00] = [this]() -> MachineCycles {
		return 1;
	};

	// LD BC, d16
	instructions_[0x01] = [this]() -> MachineCycles {
		registers_.bc = FetchWord();
		return 3;
	};

	// LD (BC), A
	instructions_[0x02] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.bc, registers_.af.GetHigh());
		return 2;
	};

	// INC BC
	instructions_[0x03] = [this]() -> MachineCycles {
		++registers_.bc;
		return 2;
	};

	// INC B
	instructions_[0x04] = [this]() -> MachineCycles {
		IncrementRegister(registers_.bc.GetHigh());
		return 1;
	};

	// DEC B
	instructions_[0x05] = [this]() -> MachineCycles {
		DecrementRegister(registers_.bc.GetHigh());
		return 1;
	};

	// LD B, d8
	instructions_[0x06] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = FetchByte();
		return 2;
	};

	// RLCA
	instructions_[0x07] = [this]() -> MachineCycles {
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() << 1) | ((registers_.af.GetHigh() & 0x80) >> 7);
		return 1;
	};

	// LD (a16), SP
	instructions_[0x08] = [this]() -> MachineCycles {
		mmu_->WriteWord(FetchWord(), registers_.sp);
		return 5;
	};

	// ADD HL, BC
	instructions_[0x09] = [this]() -> MachineCycles {
		AddToHl(registers_.bc);
		return 2;
	};

	// LD A, (BC)
	instructions_[0x0A] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(registers_.bc);
		return 2;
	};

	// DEC BC
	instructions_[0x0B] = [this]() -> MachineCycles {
		--registers_.bc;
		return 2;
	};

	// INC C
	instructions_[0x0C] = [this]() -> MachineCycles {
		IncrementRegister(registers_.bc.GetLow());
		return 1;
	};

	// DEC C
	instructions_[0x0D] = [this]() -> MachineCycles {
		DecrementRegister(registers_.bc.GetLow());
		return 1;
	};

	// LD C, d8
	instructions_[0x0E] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = FetchByte();
		return 2;
	};

	// RRCA
	instructions_[0x0F] = [this]() -> MachineCycles {
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() >> 1) | ((registers_.af.GetHigh() & 0x01) << 7);
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x10 - 0x1F
	//TODO STOP 0x10

	// LD DE, d16
	instructions_[0x11] = [this]() -> MachineCycles {
		registers_.de = FetchWord();
		return 3;
	};

	// LD (DE), A
	instructions_[0x12] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.de, registers_.af.GetHigh());
		return 2;
	};

	// INC DE
	instructions_[0x13] = [this]() -> MachineCycles {
		++registers_.de;
		return 2;
	};

	// INC D
	instructions_[0x14] = [this]() -> MachineCycles {
		IncrementRegister(registers_.de.GetHigh());
		return 1;
	};

	// DEC D
	instructions_[0x15] = [this]() -> MachineCycles {
		DecrementRegister(registers_.de.GetHigh());
		return 1;
	};

	// LD D, d8
	instructions_[0x16] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = FetchByte();
		return 2;
	};

	// RLA
	instructions_[0x17] = [this]() -> MachineCycles {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x80) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() = (registers_.af.GetHigh() << 1) | carry_value;
		return 1;
	};

	// JR r8
	instructions_[0x18] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());

		registers_.pc += displacement;
		return 3;
	};

	// ADD HL, DE
	instructions_[0x19] = [this]() -> MachineCycles {
		AddToHl(registers_.de);
		return 2;
	};

	// LD A, (DE)
	instructions_[0x1A] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(registers_.de);
		return 2;
	};

	// DEC DE
	instructions_[0x1B] = [this]() -> MachineCycles {
		--registers_.de;
		return 2;
	};

	// INC E
	instructions_[0x1C] = [this]() -> MachineCycles {
		IncrementRegister(registers_.de.GetLow());
		return 1;
	};

	// DEC E
	instructions_[0x1D] = [this]() -> MachineCycles {
		DecrementRegister(registers_.de.GetLow());
		return 1;
	};

	// LD E, d8
	instructions_[0x1E] = [this]() -> MachineCycles {
		registers_.de.GetLow() = FetchByte();
		return 2;
	};

	// RRA
	instructions_[0x1F] = [this]() -> MachineCycles {
		const auto carry_value = static_cast<uint8_t>(IsFlagSet(Flags::C) ? 1 : 0);
		ClearFlag(Flags::All);
		if ((registers_.af.GetHigh() & 0x01) != 0) SetFlag(Flags::C);
		registers_.af.GetHigh() >>= 1;
		registers_.af.GetHigh() |= (carry_value << 7);
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x20 - 0x2F
	// JR NZ, r8
	instructions_[0x20] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (!IsFlagSet(Flags::Z))
		{
			registers_.pc += displacement;
			return 3;
		}
		return 2;
	};

	// LD HL, d16
	instructions_[0x21] = [this]() -> MachineCycles {
		registers_.hl = FetchWord();
		return 3;
	};

	// LD (HL+), A
	instructions_[0x22] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl++, registers_.af.GetHigh());
		return 2;
	};

	// INC HL
	instructions_[0x23] = [this]() -> MachineCycles {
		++registers_.hl;
		return 2;
	};

	// INC H
	instructions_[0x24] = [this]() -> MachineCycles {
		IncrementRegister(registers_.hl.GetHigh());
		return 1;
	};

	// DEC H
	instructions_[0x25] = [this]() -> MachineCycles {
		DecrementRegister(registers_.hl.GetHigh());
		return 1;
	};

	// LD H, d8
	instructions_[0x26] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = FetchByte();
		return 2;
	};

	// DAA
	instructions_[0x27] = [this]() -> MachineCycles {
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

		return 1;
	};

	// JR Z, r8
	instructions_[0x28] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (IsFlagSet(Flags::Z))
		{
			registers_.pc += displacement;
			return 3;
		}
		return 2;
	};

	// ADD HL, HL
	instructions_[0x29] = [this]() -> MachineCycles {
		AddToHl(registers_.hl);
		return 2;
	};

	// LD A, (HL+)
	instructions_[0x2A] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(registers_.hl++);
		return 2;
	};

	// DEC HL
	instructions_[0x2B] = [this]() -> MachineCycles {
		--registers_.hl;
		return 2;
	};

	// INC L
	instructions_[0x2C] = [this]() -> MachineCycles {
		IncrementRegister(registers_.hl.GetLow());
		return 1;
	};

	// DEC L
	instructions_[0x2D] = [this]() -> MachineCycles {
		DecrementRegister(registers_.hl.GetLow());
		return 1;
	};

	// LD L, d8
	instructions_[0x2E] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = FetchByte();
		return 2;
	};

	// CPL
	instructions_[0x2F] = [this]() -> MachineCycles {
		registers_.af.GetHigh() ^= 0xFF;
		SetFlag(Flags::H | Flags::N);
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x30 - 0x3F
	// JR NC, r8
	instructions_[0x30] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (!IsFlagSet(Flags::C))
		{
			registers_.pc += displacement;
			return 3;
		}
		return 2;
	};

	// LD SP, d16
	instructions_[0x31] = [this]() -> MachineCycles {
		registers_.sp = FetchWord();
		return 3;
	};

	// LD (HL-), A
	instructions_[0x32] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl--, registers_.af.GetHigh());
		return 2;
	};

	// INC SP
	instructions_[0x33] = [this]() -> MachineCycles {
		++registers_.sp;
		return 2;
	};

	// INC (HL)
	instructions_[0x34] = [this]() -> MachineCycles {
		ClearFlag(Flags::Z | Flags::H | Flags::N);
		auto value = mmu_->ReadByte(registers_.hl);
		mmu_->WriteByte(registers_.hl, ++value);
		if ((value & 0x0F) == 0x00) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }
		return 3;
	};

	// DEC (HL)
	instructions_[0x35] = [this]() -> MachineCycles {
		ClearFlag(Flags::Z | Flags::H);
		SetFlag(Flags::N);
		auto value = mmu_->ReadByte(registers_.hl);
		mmu_->WriteByte(registers_.hl, --value);
		if ((value & 0x0F) == 0x0F) { SetFlag(Flags::H); }
		if (value == 0) { SetFlag(Flags::Z); }
		return 3;
	};

	// LD (HL), d8
	instructions_[0x36] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, FetchByte());
		return 3;
	};

	// SCF
	instructions_[0x37] = [this]() -> MachineCycles {
		SetFlag(Flags::C);
		ClearFlag(Flags::H | Flags::N);
		return 1;
	};

	// JR C, r8
	instructions_[0x38] = [this]() -> MachineCycles {
		const auto displacement = static_cast<int8_t>(FetchByte());
		if (IsFlagSet(Flags::C))
		{
			registers_.pc += displacement;
			return 3;
		}
		return 2;
	};

	// ADD HL, SP
	instructions_[0x39] = [this]() -> MachineCycles {
		AddToHl(registers_.sp);
		return 2;
	};

	// LD A, (HL-)
	instructions_[0x3A] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(registers_.hl--);
		return 2;
	};

	// DEC SP
	instructions_[0x3B] = [this]() -> MachineCycles {
		--registers_.sp;
		return 2;
	};

	// INC A
	instructions_[0x3C] = [this]() -> MachineCycles {
		IncrementRegister(registers_.af.GetHigh());
		return 1;
	};

	// DEC A
	instructions_[0x3D] = [this]() -> MachineCycles {
		DecrementRegister(registers_.af.GetHigh());
		return 1;
	};

	// LD A, d8
	instructions_[0x3E] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = FetchByte();
		return 2;
	};

	// CCF
	instructions_[0x3F] = [this]() -> MachineCycles {
		ToggleFlag(Flags::C);
		ClearFlag(Flags::H | Flags::N);
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x40 - 0x4F
	// LD B, B
	instructions_[0x40] = [this]() -> MachineCycles {
		return 1;
	};

	// LD B, C
	instructions_[0x41] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.bc.GetLow();
		return 1;
	};

	// LD B, D
	instructions_[0x42] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.de.GetHigh();
		return 1;
	};

	// LD B, E
	instructions_[0x43] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.de.GetLow();
		return 1;
	};

	// LD B, H
	instructions_[0x44] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.hl.GetHigh();
		return 1;
	};

	// LD B, L
	instructions_[0x45] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.hl.GetLow();
		return 1;
	};

	// LD B, (HL)
	instructions_[0x46] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD B, A
	instructions_[0x47] = [this]() -> MachineCycles {
		registers_.bc.GetHigh() = registers_.af.GetHigh();
		return 1;
	};

	// LD C, B
	instructions_[0x48] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.bc.GetHigh();
		return 1;
	};

	// LD C, C
	instructions_[0x49] = [this]() -> MachineCycles {
		return 1;
	};

	// LD C, D
	instructions_[0x4A] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.de.GetHigh();
		return 1;
	};

	// LD C, E
	instructions_[0x4B] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.de.GetLow();
		return 1;
	};

	// LD C, H
	instructions_[0x4C] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.hl.GetHigh();
		return 1;
	};

	// LD C, L
	instructions_[0x4D] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.hl.GetLow();
		return 1;
	};

	// LD C, (HL)
	instructions_[0x4E] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD C, A
	instructions_[0x4F] = [this]() -> MachineCycles {
		registers_.bc.GetLow() = registers_.af.GetHigh();
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x50 - 0x5F
	// LD D, B
	instructions_[0x50] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.bc.GetHigh();
		return 1;
	};

	// LD D, C
	instructions_[0x51] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.bc.GetLow();
		return 1;
	};

	// LD D, D
	instructions_[0x52] = [this]() -> MachineCycles {
		return 1;
	};

	// LD D, E
	instructions_[0x53] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.de.GetLow();
		return 1;
	};

	// LD D, H
	instructions_[0x54] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.hl.GetHigh();
		return 1;
	};

	// LD D, L
	instructions_[0x55] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.hl.GetLow();
		return 1;
	};

	// LD D, (HL)
	instructions_[0x56] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD D, A
	instructions_[0x57] = [this]() -> MachineCycles {
		registers_.de.GetHigh() = registers_.af.GetHigh();
		return 1;
	};

	// LD E, B
	instructions_[0x58] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.bc.GetHigh();
		return 1;
	};

	// LD E, C
	instructions_[0x59] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.bc.GetLow();
		return 1;
	};

	// LD E, D
	instructions_[0x5A] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.de.GetHigh();
		return 1;
	};

	// LD E, E
	instructions_[0x5B] = [this]() -> MachineCycles {
		return 1;
	};

	// LD E, H
	instructions_[0x5C] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.hl.GetHigh();
		return 1;
	};

	// LD E, L
	instructions_[0x5D] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.hl.GetLow();
		return 1;
	};

	// LD E, (HL)
	instructions_[0x5E] = [this]() -> MachineCycles {
		registers_.de.GetLow() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD E, A
	instructions_[0x5F] = [this]() -> MachineCycles {
		registers_.de.GetLow() = registers_.af.GetHigh();
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x60 - 0x6F
	// LD H, B
	instructions_[0x60] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.bc.GetHigh();
		return 1;
	};

	// LD H, C
	instructions_[0x61] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.bc.GetLow();
		return 1;
	};

	// LD H, D
	instructions_[0x62] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.de.GetHigh();
		return 1;
	};

	// LD H, E
	instructions_[0x63] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.de.GetLow();
		return 1;
	};

	// LD H, H
	instructions_[0x64] = [this]() -> MachineCycles {
		return 1;
	};

	// LD H, L
	instructions_[0x65] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.hl.GetLow();
		return 1;
	};

	// LD H, (HL)
	instructions_[0x66] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD H, A
	instructions_[0x67] = [this]() -> MachineCycles {
		registers_.hl.GetHigh() = registers_.af.GetHigh();
		return 1;
	};

	// LD L, B
	instructions_[0x68] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.bc.GetHigh();
		return 1;
	};

	// LD L, C
	instructions_[0x69] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.bc.GetLow();
		return 1;
	};

	// LD L, D
	instructions_[0x6A] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.de.GetHigh();
		return 1;
	};

	// LD L, E
	instructions_[0x6B] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.de.GetLow();
		return 1;
	};

	// LD L, H
	instructions_[0x6C] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.hl.GetHigh();
		return 1;
	};

	// LD L, L
	instructions_[0x6D] = [this]() -> MachineCycles {
		return 1;
	};

	// LD L, (HL)
	instructions_[0x6E] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD L, A
	instructions_[0x6F] = [this]() -> MachineCycles {
		registers_.hl.GetLow() = registers_.af.GetHigh();
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x70 - 0x7F
	// LD (HL), B
	instructions_[0x70] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.bc.GetHigh());
		return 2;
	};

	// LD (HL), C
	instructions_[0x71] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.bc.GetLow());
		return 2;
	};

	// LD (HL), D
	instructions_[0x72] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.de.GetHigh());
		return 2;
	};

	// LD (HL), E
	instructions_[0x73] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.de.GetLow());
		return 2;
	};

	// LD (HL), H
	instructions_[0x74] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.hl.GetHigh());
		return 2;
	};

	// LD (HL), L
	instructions_[0x75] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.hl.GetLow());
		return 2;
	};

	// HALT
	/*instructions_[0x76] = [this]() -> MachineCycles {
		current_state_ = State::Halted;
		return 1;
	};*/

	// LD (HL), A
	instructions_[0x77] = [this]() -> MachineCycles {
		mmu_->WriteByte(registers_.hl, registers_.af.GetHigh());
		return 2;
	};

	// LD A, B
	instructions_[0x78] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.bc.GetHigh();
		return 1;
	};

	// LD A, C
	instructions_[0x79] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.bc.GetLow();
		return 1;
	};

	// LD A, D
	instructions_[0x7A] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.de.GetHigh();
		return 1;
	};

	// LD A, E
	instructions_[0x7B] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.de.GetLow();
		return 1;
	};

	// LD A, H
	instructions_[0x7C] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.hl.GetHigh();
		return 1;
	};

	// LD A, L
	instructions_[0x7D] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = registers_.hl.GetLow();
		return 1;
	};

	// LD A, (HL)
	instructions_[0x7E] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(registers_.hl);
		return 2;
	};

	// LD A, A
	instructions_[0x7F] = [this]() -> MachineCycles {
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x80 - 0x8F
	// ADD A, B
	instructions_[0x80] = [this]() -> MachineCycles {
		Add(registers_.bc.GetHigh());
		return 1;
	};

	// ADD A, C
	instructions_[0x81] = [this]() -> MachineCycles {
		Add(registers_.bc.GetLow());
		return 1;
	};

	// ADD A, D
	instructions_[0x82] = [this]() -> MachineCycles {
		Add(registers_.de.GetHigh());
		return 1;
	};

	// ADD A, E
	instructions_[0x83] = [this]() -> MachineCycles {
		Add(registers_.de.GetLow());
		return 1;
	};

	// ADD A, H
	instructions_[0x84] = [this]() -> MachineCycles {
		Add(registers_.hl.GetHigh());
		return 1;
	};

	// ADD A, L
	instructions_[0x85] = [this]() -> MachineCycles {
		Add(registers_.hl.GetLow());
		return 1;
	};

	// ADD (HL)
	instructions_[0x86] = [this]() -> MachineCycles {
		Add(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// ADD A, A
	instructions_[0x87] = [this]() -> MachineCycles {
		Add(registers_.af.GetHigh());
		return 1;
	};

	// ADC A, B
	instructions_[0x88] = [this]() -> MachineCycles {
		Adc(registers_.bc.GetHigh());
		return 1;
	};

	// ADC A, C
	instructions_[0x89] = [this]() -> MachineCycles {
		Adc(registers_.bc.GetLow());
		return 1;
	};

	// ADC A, D
	instructions_[0x8A] = [this]() -> MachineCycles {
		Adc(registers_.de.GetHigh());
		return 1;
	};

	// ADC A, E
	instructions_[0x8B] = [this]() -> MachineCycles {
		Adc(registers_.de.GetLow());
		return 1;
	};

	// ADC A, H
	instructions_[0x8C] = [this]() -> MachineCycles {
		Adc(registers_.hl.GetHigh());
		return 1;
	};

	// ADC A, L
	instructions_[0x8D] = [this]() -> MachineCycles {
		Adc(registers_.hl.GetLow());
		return 1;
	};

	// ADC A, (HL)
	instructions_[0x8E] = [this]() -> MachineCycles {
		Adc(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// ADC A, A
	instructions_[0x8F] = [this]() -> MachineCycles {
		Adc(registers_.af.GetHigh());
		return 1;
	};
#pragma endregion

#pragma region Instructions 0x90 - 0x9F
	// SUB B
	instructions_[0x90] = [this]() -> MachineCycles {
		Sub(registers_.bc.GetHigh());
		return 1;
	};

	// SUB C
	instructions_[0x91] = [this]() -> MachineCycles {
		Sub(registers_.bc.GetLow());
		return 1;
	};

	// SUB D
	instructions_[0x92] = [this]() -> MachineCycles {
		Sub(registers_.de.GetHigh());
		return 1;
	};

	// SUB E
	instructions_[0x93] = [this]() -> MachineCycles {
		Sub(registers_.de.GetLow());
		return 1;
	};

	// SUB H
	instructions_[0x94] = [this]() -> MachineCycles {
		Sub(registers_.hl.GetHigh());
		return 1;
	};

	// SUB L
	instructions_[0x95] = [this]() -> MachineCycles {
		Sub(registers_.hl.GetLow());
		return 1;
	};

	// SUB (HL)
	instructions_[0x96] = [this]() -> MachineCycles {
		Sub(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// SUB A
	instructions_[0x97] = [this]() -> MachineCycles {
		Sub(registers_.af.GetHigh());
		return 1;
	};

	// SBC A, B
	instructions_[0x98] = [this]() -> MachineCycles {
		Sbc(registers_.bc.GetHigh());
		return 1;
	};

	// SBC A, C
	instructions_[0x99] = [this]() -> MachineCycles {
		Sbc(registers_.bc.GetLow());
		return 1;
	};

	// SBC A, D
	instructions_[0x9A] = [this]() -> MachineCycles {
		Sbc(registers_.de.GetHigh());
		return 1;
	};

	// SBC A, E
	instructions_[0x9B] = [this]() -> MachineCycles {
		Sbc(registers_.de.GetLow());
		return 1;
	};

	// SBC A, H
	instructions_[0x9C] = [this]() -> MachineCycles {
		Sbc(registers_.hl.GetHigh());
		return 1;
	};

	// SBC A, L
	instructions_[0x9D] = [this]() -> MachineCycles {
		Sbc(registers_.hl.GetLow());
		return 1;
	};

	// SBC A, (HL)
	instructions_[0x9E] = [this]() -> MachineCycles {
		Sbc(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// SBC A, A
	instructions_[0x9F] = [this]() -> MachineCycles {
		Sbc(registers_.af.GetHigh());
		return 1;
	};
#pragma endregion

#pragma region Instructions 0xA0 - 0xAF
	// AND B
	instructions_[0xA0] = [this]() -> MachineCycles {
		And(registers_.bc.GetHigh());
		return 1;
	};

	// AND C
	instructions_[0xA1] = [this]() -> MachineCycles {
		And(registers_.bc.GetLow());
		return 1;
	};

	// AND D
	instructions_[0xA2] = [this]() -> MachineCycles {
		And(registers_.de.GetHigh());
		return 1;
	};

	// AND E
	instructions_[0xA3] = [this]() -> MachineCycles {
		And(registers_.de.GetLow());
		return 1;
	};

	// AND H
	instructions_[0xA4] = [this]() -> MachineCycles {
		And(registers_.hl.GetHigh());
		return 1;
	};

	// AND L
	instructions_[0xA5] = [this]() -> MachineCycles {
		And(registers_.hl.GetLow());
		return 1;
	};

	// AND (HL)
	instructions_[0xA6] = [this]() -> MachineCycles {
		And(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// AND A
	instructions_[0xA7] = [this]() -> MachineCycles {
		And(registers_.af.GetHigh());
		return 1;
	};

	// XOR B
	instructions_[0xA8] = [this]() -> MachineCycles {
		Xor(registers_.bc.GetHigh());
		return 1;
	};

	// XOR C
	instructions_[0xA9] = [this]() -> MachineCycles {
		Xor(registers_.bc.GetLow());
		return 1;
	};

	// XOR D
	instructions_[0xAA] = [this]() -> MachineCycles {
		Xor(registers_.de.GetHigh());
		return 1;
	};

	// XOR E
	instructions_[0xAB] = [this]() -> MachineCycles {
		Xor(registers_.de.GetLow());
		return 1;
	};

	// XOR H
	instructions_[0xAC] = [this]() -> MachineCycles {
		Xor(registers_.hl.GetHigh());
		return 1;
	};

	// XOR L
	instructions_[0xAD] = [this]() -> MachineCycles {
		Xor(registers_.hl.GetLow());
		return 1;
	};

	// XOR (HL)
	instructions_[0xAE] = [this]() -> MachineCycles {
		Xor(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// XOR A
	instructions_[0xAF] = [this]() -> MachineCycles {
		Xor(registers_.af.GetHigh());
		return 1;
	};
#pragma endregion

#pragma region Instructions 0xB0 - 0xBF
	// OR B
	instructions_[0xB0] = [this]() -> MachineCycles {
		Or(registers_.bc.GetHigh());
		return 1;
	};

	// OR C
	instructions_[0xB1] = [this]() -> MachineCycles {
		Or(registers_.bc.GetLow());
		return 1;
	};

	// OR D
	instructions_[0xB2] = [this]() -> MachineCycles {
		Or(registers_.de.GetHigh());
		return 1;
	};

	// OR E
	instructions_[0xB3] = [this]() -> MachineCycles {
		Or(registers_.de.GetLow());
		return 1;
	};

	// OR H
	instructions_[0xB4] = [this]() -> MachineCycles {
		Or(registers_.hl.GetHigh());
		return 1;
	};

	// OR L
	instructions_[0xB5] = [this]() -> MachineCycles {
		Or(registers_.hl.GetLow());
		return 1;
	};

	// OR (HL)
	instructions_[0xB6] = [this]() -> MachineCycles {
		Or(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// OR A
	instructions_[0xB7] = [this]() -> MachineCycles {
		Or(registers_.af.GetHigh());
		return 1;
	};

	// CP B
	instructions_[0xB8] = [this]() -> MachineCycles {
		Compare(registers_.bc.GetHigh());
		return 1;
	};

	// CP C
	instructions_[0xB9] = [this]() -> MachineCycles {
		Compare(registers_.bc.GetLow());
		return 1;
	};

	// CP D
	instructions_[0xBA] = [this]() -> MachineCycles {
		Compare(registers_.de.GetHigh());
		return 1;
	};

	// CP E
	instructions_[0xBB] = [this]() -> MachineCycles {
		Compare(registers_.de.GetLow());
		return 1;
	};

	// CP H
	instructions_[0xBC] = [this]() -> MachineCycles {
		Compare(registers_.hl.GetHigh());
		return 1;
	};

	// CP L
	instructions_[0xBD] = [this]() -> MachineCycles {
		Compare(registers_.hl.GetLow());
		return 1;
	};

	// CP (HL)
	instructions_[0xBE] = [this]() -> MachineCycles {
		Compare(mmu_->ReadByte(registers_.hl));
		return 2;
	};

	// CP A
	instructions_[0xBF] = [this]() -> MachineCycles {
		Compare(registers_.af.GetHigh());
		return 1;
	};
#pragma endregion

#pragma region Instructions 0xC0 - 0xCF
	// RET NZ
	instructions_[0xC0] = [this]() -> MachineCycles {
		if (!IsFlagSet(Flags::Z))
		{
			Return();
			return 5;
		}
		return 2;
	};

	// POP BC
	instructions_[0xC1] = [this]() -> MachineCycles {
		registers_.bc = ReadWordFromStack();
		return 3;
	};

	// JP NZ, a16
	instructions_[0xC2] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::Z))
		{
			registers_.pc = address;
			return 4;
		}
		return 3;
	};

	// JP a16
	instructions_[0xC3] = [this]() -> MachineCycles {
		registers_.pc = FetchWord();
		return 4;
	};

	// CALL NZ, a16
	instructions_[0xC4] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::Z))
		{
			Call(address);
			return 6;
		}
		return 3;
	};

	// PUSH BC
	instructions_[0xC5] = [this]() -> MachineCycles {
		WriteWordToStack(registers_.bc);
		return 4;
	};

	// ADD A, d8
	instructions_[0xC6] = [this]() -> MachineCycles {
		Add(FetchByte());
		return 2;
	};

	// RST 00H
	instructions_[0xC7] = [this]() -> MachineCycles {
		Call(0x0000);
		return 4;
	};

	// RET Z
	instructions_[0xC8] = [this]() -> MachineCycles {
		if (IsFlagSet(Flags::Z))
		{
			Return();
			return 5;
		}
		return 2;
	};

	// RET
	instructions_[0xC9] = [this]() -> MachineCycles {
		Return();
		return 4;
	};

	// JP Z, a16
	instructions_[0xCA] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::Z))
		{
			registers_.pc = address;
			return 4;
		}
		return 3;
	};

	// PREFIX CB
	instructions_[0xCB] = [this]() -> MachineCycles {
		return cb_instructions_[FetchByte()]();
	};

	// CALL Z, a16
	instructions_[0xCC] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::Z))
		{
			Call(address);
			return 6;
		}
		return 3;
	};

	// CALL a16
	instructions_[0xCD] = [this]() -> MachineCycles {
		Call(FetchWord());
		return 6;
	};

	// ADC A, d8
	instructions_[0xCE] = [this]() -> MachineCycles {
		Adc(FetchByte());
		return 2;
	};

	// RST 08H
	instructions_[0xCF] = [this]() -> MachineCycles {
		Call(0x0008);
		return 4;
	};
#pragma endregion

#pragma region Instructions 0xD0 - 0xDF
	// RET NC
	instructions_[0xD0] = [this]() -> MachineCycles {
		if (!IsFlagSet(Flags::C))
		{
			Return();
			return 5;
		}
		return 2;
	};

	// POP DE
	instructions_[0xD1] = [this]() -> MachineCycles {
		registers_.de = ReadWordFromStack();
		return 3;
	};

	// JP NC, a16
	instructions_[0xD2] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::C))
		{
			registers_.pc = address;
			return 4;
		}
		return 3;
	};

	// CALL NC, a16
	instructions_[0xD4] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (!IsFlagSet(Flags::C))
		{
			Call(address);
			return 6;
		}
		return 3;
	};

	// PUSH DE
	instructions_[0xD5] = [this]() -> MachineCycles {
		WriteWordToStack(registers_.de);
		return 4;
	};

	// SUB A, d8
	instructions_[0xD6] = [this]() -> MachineCycles {
		Sub(FetchByte());
		return 2;
	};

	// RST 10H
	instructions_[0xD7] = [this]() -> MachineCycles {
		Call(0x0010);
		return 4;
	};

	// RET C
	instructions_[0xD8] = [this]() -> MachineCycles {
		if (IsFlagSet(Flags::C))
		{
			Return();
			return 5;
		}
		return 2;
	};

	// RETI
	instructions_[0xD9] = [this]() -> MachineCycles {
		Return();
		//TODO: is the enabling immediate? Or does it take 1 additional instruction as for EI?
		interrupt_master_enable_ = true;
		return 4;
	};

	// JP C, a16
	instructions_[0xDA] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::C))
		{
			registers_.pc = address;
			return 4;
		}
		return 3;
	};

	// CALL C, a16
	instructions_[0xDC] = [this]() -> MachineCycles {
		const auto address = FetchWord();
		if (IsFlagSet(Flags::C))
		{
			Call(address);
			return 6;
		}
		return 3;
	};

	// SBC A, d8
	instructions_[0xDE] = [this]() -> MachineCycles {
		Sbc(FetchByte());
		return 2;
	};

	// RST 18H
	instructions_[0xDF] = [this]() -> MachineCycles {
		Call(0x0018);
		return 4;
	};
#pragma endregion

#pragma region Instructions 0xE0 - 0xEF
	// LDH (a8), A
	instructions_[0xE0] = [this]() -> MachineCycles {
		mmu_->WriteByte(Memory::io_region_start_ + FetchByte(), registers_.af.GetHigh());
		return 3;
	};

	// POP HL
	instructions_[0xE1] = [this]() -> MachineCycles {
		registers_.hl = ReadWordFromStack();
		return 3;
	};

	// LD (C), A
	instructions_[0xE2] = [this]() -> MachineCycles {
		mmu_->WriteByte(Memory::io_region_start_ + registers_.bc.GetLow(), registers_.af.GetHigh());
		return 2;
	};

	// PUSH HL
	instructions_[0xE5] = [this]() -> MachineCycles {
		WriteWordToStack(registers_.hl);
		return 4;
	};

	// AND d8
	instructions_[0xE6] = [this]() -> MachineCycles {
		And(FetchByte());
		return 2;
	};

	// RST 20H
	instructions_[0xE7] = [this]() -> MachineCycles {
		Call(0x0020);
		return 4;
	};

	// ADD SP, r8
	instructions_[0xE8] = [this]() -> MachineCycles {
		ClearFlag(Flags::All);
		const auto displacement = static_cast<int8_t>(FetchByte());
		// Flags are calculated on the lower byte of SP, adding an unsigned displacement
		if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
		if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
		registers_.sp += displacement;
		return 4;
	};

	// JP (HL)
	instructions_[0xE9] = [this]() -> MachineCycles {
		registers_.pc = registers_.hl;
		return 1;
	};

	// LD (a16), A
	instructions_[0xEA] = [this]() -> MachineCycles {
		mmu_->WriteByte(FetchWord(), registers_.af.GetHigh());
		return 4;
	};

	// XOR d8
	instructions_[0xEE] = [this]() -> MachineCycles {
		Xor(FetchByte());
		return 2;
	};

	// RST 28H
	instructions_[0xEF] = [this]() -> MachineCycles {
		Call(0x0028);
		return 4;
	};
#pragma endregion

#pragma region Instructions 0xF0 - 0xFF
	// LDH A, (a8)
	instructions_[0xF0] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(Memory::io_region_start_ + FetchByte());
		return 3;
	};

	// POP AF
	instructions_[0xF1] = [this]() -> MachineCycles {
		registers_.af = ReadWordFromStack() & 0xFFF0;
		return 3;
	};

	// LD A, (C)
	instructions_[0xF2] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(Memory::io_region_start_ + registers_.bc.GetLow());
		return 2;
	};

	// DI
	instructions_[0xF3] = [this]() -> MachineCycles {
		interrupt_master_enable_ = false;
		return 1;
	};

	// PUSH AF
	instructions_[0xF5] = [this]() -> MachineCycles {
		WriteWordToStack(registers_.af);
		return 4;
	};

	// OR d8
	instructions_[0xF6] = [this]() -> MachineCycles {
		Or(FetchByte());
		return 2;
	};

	// RST 30H
	instructions_[0xF7] = [this]() -> MachineCycles {
		Call(0x0030);
		return 4;
	};

	// LD HL, SP+r8
	instructions_[0xF8] = [this]() -> MachineCycles {
		ClearFlag(Flags::All);
		const auto displacement = static_cast<int8_t>(FetchByte());
		// Flags are calculated on the lower byte of SP, adding an unsigned displacement
		if (((registers_.sp & 0x000F) + (static_cast<uint8_t>(displacement) & 0x0F)) > 0x0F) SetFlag(Flags::H);
		if (((registers_.sp & 0x00FF) + static_cast<uint8_t>(displacement)) > 0xFF) SetFlag(Flags::C);
		registers_.hl = registers_.sp + displacement;
		return 3;
	};

	// LD SP, HL
	instructions_[0xF9] = [this]() -> MachineCycles {
		registers_.sp = registers_.hl;
		return 2;
	};

	// LD A, (a16)
	instructions_[0xFA] = [this]() -> MachineCycles {
		registers_.af.GetHigh() = mmu_->ReadByte(FetchWord());
		return 4;
	};

	// EI
	instructions_[0xFB] = [this]() -> MachineCycles {
		ime_requested_ = true;
		return 1;
	};

	// CP d8
	instructions_[0xFE] = [this]() -> MachineCycles {
		Compare(FetchByte());
		return 2;
	};

	// RST 38H
	instructions_[0xFF] = [this]() -> MachineCycles {
		Call(0x0038);
		return 4;
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
