#include "Timer.h"
#include "MMU.h"

namespace jb
{

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
		// If TIMA was written right when it overflowed, the interrupt does not happen
		if (timer_counter_ == 0)
		{
			timer_counter_ = timer_modulo_;
			mmu_->WriteByte(Memory::TIMA, timer_counter_, false);
			mmu_->SetBit<2>(Memory::IF);
			timer_overflow_state_ = TimerOverflowState::JustReloaded;
		}
		else
		{
			timer_overflow_state_ = TimerOverflowState::NoOverflow;
		}
		break;
	case TimerOverflowState::JustReloaded:
		timer_overflow_state_ = TimerOverflowState::NoOverflow;
		break;
	case TimerOverflowState::NoOverflow:
	default:
		break;
	}

	internal_counter_ += 4;

	if ((internal_counter_ & (divider_period_ - 1)) == 0)
	{
		mmu_->WriteByte(Memory::DIV, (internal_counter_ >> 8), false);
	}

	if (timer_enabled_ && ((internal_counter_ & (timer_period_ - 1)) == 0))
	{
		IncreaseTimer();
	}
}

void Timer::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	switch (address)
	{
	case Memory::DIV:
		{const auto previous_internal_counter = internal_counter_;

		// Writing any value to this register resets it to 0x00
		internal_counter_ = 0;
		mmu_->WriteByte(Memory::DIV, 0, false);

		// Obscure timer behavior
		if (timer_enabled_ && (previous_internal_counter & (timer_period_ >> 1)) != 0)
		{
			IncreaseTimer();
		}}
		break;
	case Memory::TIMA:
		// Timer overflow obscure behavior: the cycle when TIMA is reloaded with TMA, writing to TIMA has no effect (TMA prevails)
		if (timer_overflow_state_ == TimerOverflowState::JustReloaded)
		{
			mmu_->WriteByte(Memory::TIMA, timer_counter_, false);
			break;
		}

		timer_counter_ = value;
		break;
	case Memory::TMA:
		timer_modulo_ = value;

		// Timer overflow obscure behavior: the cycle when TIMA is reloaded with TMA, writing to TMA has effect and the new TMA will be loaded into TIMA
		if (timer_overflow_state_ == TimerOverflowState::JustReloaded)
		{
			timer_counter_ = timer_modulo_;
			mmu_->WriteByte(Memory::TIMA, timer_counter_, false);
		}
		break;
	case Memory::TAC:
		{const auto previous_timer_enabled = timer_enabled_;
		const auto previous_timer_period = timer_period_;

		timer_enabled_ = (value & (1 << 2)) != 0;
		timer_period_ = timer_periods_map_[(value & 0x03)];

		// Obscure timer behavior
		const auto falling_edge_detected = ((previous_timer_enabled && ((internal_counter_ & (previous_timer_period >> 1)) != 0))
			&& (!timer_enabled_ || ((internal_counter_ & (timer_period_ >> 1)) == 0)));
		if (falling_edge_detected)
		{
			IncreaseTimer();
		}}
		break;
	}
}

void Timer::IncreaseTimer()
{
	if (++timer_counter_ == 0)
	{
		timer_overflow_state_ = TimerOverflowState::JustOverflowed;
	}
	mmu_->WriteByte(Memory::TIMA, timer_counter_, false);
}

}
