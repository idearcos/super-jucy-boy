#pragma once

#include "Memory.h"
#include "CPU.h"
#include <array>

class MMU;

class Timer final : public CPU::Listener
{
public:
	Timer(MMU &mmu);
	~Timer() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse() override;

	// MMU mapped memory read/write functions
	uint8_t OnIoMemoryRead(Memory::Address address) const;
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	template<class Archive>
	void serialize(Archive &archive);

private:
	void IncreaseTimer();

private:
	// Divider logic
	uint16_t internal_counter_{ 0 };
	const size_t divider_period_{ 256 };

	// Timer logic
	uint8_t timer_counter_{ 0 };
	const std::array<size_t, 4> timer_periods_map_{ 1024, 16, 64, 256 };
	size_t timer_period_{ timer_periods_map_[0] };
	uint8_t timer_modulo_{ 0 };
	bool timer_enabled_{ false };

	// Timer overflow obscure behavior
	enum class TimerOverflowState
	{
		JustOverflowed,
		JustReloaded,
		NoOverflow
	} timer_overflow_state_{ TimerOverflowState::NoOverflow };

	MMU* mmu_;
};

template<class Archive>
void Timer::serialize(Archive &archive)
{
	archive(internal_counter_, timer_counter_, timer_period_, timer_modulo_, timer_enabled_, timer_overflow_state_);
}
