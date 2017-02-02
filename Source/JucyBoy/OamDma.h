#pragma once

#include "CPU.h"

class MMU;

class OamDma final : public CPU::Listener
{
public:
	OamDma(MMU &mmu);
	~OamDma() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse() override;

	// MMU listener functions
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

private:
	enum class State
	{
		Startup, // 1 cycle
		Active, // 160 cycles
		Teardown, // 1 cycle
		Inactive
	};
	State oam_dma_state_{ State::Inactive };
	Memory::Address oam_dma_source_{ 0x0000 };
	uint8_t current_oam_dma_byte_index_{ 0 };

	MMU* mmu_;
};
