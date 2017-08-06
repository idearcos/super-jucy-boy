#include "Timer.h"
#include "MMU.h"

Timer::Timer(MMU &mmu) :
	mmu_{ &mmu }
{

}

void Timer::OnMachineCycleLapse()
{
	// Timer overflow obscure behavior
	switch (timer_overflow_state_)
	{
	case TimerOverflowState::JustOverflowed:
		timer_counter_ = timer_modulo_;
		timer_overflow_state_ = TimerOverflowState::JustReloaded;
		break;
	case TimerOverflowState::JustReloaded:
		timer_overflow_state_ = TimerOverflowState::NoOverflow;
		break;
	case TimerOverflowState::NoOverflow:
	default:
		break;
	}

	internal_counter_ += 4;

	if (timer_enabled_ && ((internal_counter_ & (timer_period_ - 1)) == 0))
	{
		IncreaseTimer();
	}
}

// MMU mapped memory read/write functions
uint8_t Timer::OnIoMemoryRead(const Memory::Address &address) const
{
	switch (static_cast<uint16_t>(address))
	{
	case Memory::DIV:
		return (internal_counter_ >> 8);
	case Memory::TIMA:
		return timer_counter_;
	case Memory::TMA:
		return timer_modulo_;
	case Memory::TAC:
		{uint8_t timer_period_code{ 0 };
		switch (timer_period_)
		{
		case 1024:
			timer_period_code = 0;
			break;
		case 16:
			timer_period_code = 1;
			break;
		case 64:
			timer_period_code = 2;
			break;
		case 256:
			timer_period_code = 3;
			break;
		default:
			throw std::logic_error{ "Invalid timer period value: " + timer_period_ };
		}
		return 0xF8 | (timer_enabled_ << 2) | timer_period_code; }
	default:
		throw std::invalid_argument{ "Reading from invalid memory address in Timer: " + static_cast<uint16_t>(address) };
	}
}

void Timer::OnIoMemoryWritten(const Memory::Address &address, uint8_t value)
{
	switch (static_cast<uint16_t>(address))
	{
	case Memory::DIV:
		{const auto previous_internal_counter = internal_counter_;

		// Writing any value to this register resets it to 0x00
		internal_counter_ = 0;

		// Obscure timer behavior
		if (timer_enabled_ && (previous_internal_counter & (timer_period_ >> 1)) != 0)
		{
			IncreaseTimer();
		}}
		break;
	case Memory::TIMA:
		// Timer overflow obscure behavior: the cycle when TIMA is reloaded with TMA, writing to TIMA has no effect (TMA prevails)
		if (timer_overflow_state_ == TimerOverflowState::JustReloaded) break;

		// If TIMA was written right when it overflowed, the interrupt does not happen
		if (timer_overflow_state_ == TimerOverflowState::JustOverflowed)
		{
			timer_overflow_state_ = TimerOverflowState::NoOverflow;
		}

		timer_counter_ = value;
		break;
	case Memory::TMA:
		timer_modulo_ = value;

		// Timer overflow obscure behavior: the cycle when TIMA is reloaded with TMA, writing to TMA has effect and the new TMA will be loaded into TIMA
		if (timer_overflow_state_ == TimerOverflowState::JustReloaded)
		{
			timer_counter_ = timer_modulo_;
		}
		break;
	case Memory::TAC:
		{const auto previous_timer_enabled = timer_enabled_;
		const auto previous_timer_period = timer_period_;

		timer_enabled_ = (value & 0x04) != 0;
		timer_period_ = timer_periods_map_[(value & 0x03)];

		// Obscure timer behavior
		const auto falling_edge_detected = ((previous_timer_enabled && ((internal_counter_ & (previous_timer_period >> 1)) != 0))
			&& (!timer_enabled_ || ((internal_counter_ & (timer_period_ >> 1)) == 0)));
		if (falling_edge_detected)
		{
			IncreaseTimer();
		}}
		break;
	default:
		throw std::invalid_argument{ "Writing to invalid memory address in Timer: " + static_cast<uint16_t>(address) };
	}
}

void Timer::IncreaseTimer()
{
	if (++timer_counter_ == 0)
	{
		timer_overflow_state_ = TimerOverflowState::JustOverflowed;
		mmu_->SetBit(Memory::IF, 2);
	}
}
