#pragma once
#include "Memory.h"
#include "CPU.h"
#include <array>

class MMU;

namespace jb
{

class Timer final : public CPU::Listener
{
public:
	Timer(MMU &mmu);
	~Timer() = default;

	// CPU::Listener overrides
	void OnCyclesLapsed(CPU::MachineCycles cycles) override;

	// MMU listener functions
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

private:
	void LapseCyclesOnDivider(CPU::MachineCycles cycles);
	void LapseCyclesOnTimer(CPU::MachineCycles cycles);

private:
	// Divider logic
	CPU::MachineCycles cpu_cycles_lapsed_on_divider_{ 0 };
	const size_t divider_period_{ 64 };
	uint8_t divider_counter_{ 0 };

	// Timer logic
	CPU::MachineCycles cpu_cycles_lapsed_on_timer_{ 0 };
	const std::array<size_t, 4> timer_periods_map_{ 256, 4, 16, 64 };
	size_t timer_period_{ timer_periods_map_[0] };
	uint8_t timer_counter_{ 0 };
	uint8_t timer_modulo_{ 0 };

	bool timer_enabled_{ false };

	MMU* mmu_;
};

}
