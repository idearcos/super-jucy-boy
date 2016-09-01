#include "CPU.h"

#include "CPU.h"
#include <sstream>
#include <iomanip>

void CPU::PopulateCbInstructions()
{
	for (size_t ii = 0; ii < instructions_.size(); ++ii)
	{
		cb_instructions_[ii] = [this, ii]() -> MachineCycles {
			std::stringstream error;
			error << "CB Instruction not yet implemented: 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii
				<< " [" << cb_instruction_names_[ii] << "]" << std::endl
				<< "(PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << registers_.pc - 1 << ")";
			throw std::runtime_error(error.str());
		};
	}
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
