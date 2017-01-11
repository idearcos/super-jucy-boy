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
	LapseMachineCycleOnDivider();
	LapseMachineCycleOnTimer();
}

void Timer::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	switch (address)
	{
	case Memory::divider_register_:
		// Writing any value to this register resets it to 0x00
		divider_counter_ = 0;
		cpu_cycles_lapsed_on_divider_ = 0;
		break;
	case Memory::timer_counter_register_:
		timer_counter_ = value;
		break;
	case Memory::timer_modulo_register_:
		timer_modulo_ = value;
		break;
	case Memory::timer_control_register_:
		timer_enabled_ = (value & (1 << 2)) != 0;
		timer_period_ = timer_periods_map_[(value & 0x03)];
		break;
	}
}

void Timer::LapseMachineCycleOnDivider()
{
	cpu_cycles_lapsed_on_divider_ += 1;
	if (cpu_cycles_lapsed_on_divider_ < divider_period_) return;

	cpu_cycles_lapsed_on_divider_ -= divider_period_;
	mmu_->WriteByte(Memory::divider_register_, ++divider_counter_, false);
}

void Timer::LapseMachineCycleOnTimer()
{
	if (!timer_enabled_) return;

	cpu_cycles_lapsed_on_timer_ += 1;
	if (cpu_cycles_lapsed_on_timer_ < timer_period_) return;

	cpu_cycles_lapsed_on_timer_ -= timer_period_;
	if (++timer_counter_ == 0)
	{
		timer_counter_ = timer_modulo_;
		mmu_->SetBit<2>(Memory::interrupt_flags_register_);
	}
	mmu_->WriteByte(Memory::timer_counter_register_, timer_counter_, false);
}

}
