#include "NoiseChannel.h"
#include <stdexcept>

size_t NoiseChannel::GetSample() const
{
	return enabled_ * (~lfsr_ & 0x01) * envelope_.current_volume;
}

void NoiseChannel::Reset()
{
	clock_divider_.Reset();
}

void NoiseChannel::OnClockCyclesLapsed(size_t num_clock_cycles)
{
	if (!enabled_) return;

	clock_divider_.OnInputClockCyclesLapsed(num_clock_cycles);
}

void NoiseChannel::OnClockDividerTicked()
{
	if (!enabled_) return;
	if (divisor_left_shift_ >= 14) return;

	const auto xored_low_bits_value{ (lfsr_ ^ (lfsr_ >> 1)) & 0x01 };
	lfsr_ = (lfsr_ >> 1) & ~0x4000 | (xored_low_bits_value << 14);
	if (seven_bits_lfsr_)
	{
		lfsr_ = lfsr_ & ~0x0040 | (xored_low_bits_value << 6);
	}
}

void NoiseChannel::ClockLengthCounter()
{
	if (length_counter_enabled_ && (length_counter_ > 0))
	{
		if (--length_counter_ == 0)
		{
			enabled_ = false;
		}
	}
}

void NoiseChannel::ClockVolumeEnvelope()
{
	if (!enabled_) return;
	if (!envelope_.active) return;
	if (envelope_.period == 0) return;

	if (--envelope_.cycles_left == 0)
	{
		switch (envelope_.direction)
		{
		case Envelope::Direction::Attenuate:
			if (envelope_.current_volume > 0)
			{
				if (--envelope_.current_volume == 0)
				{
					envelope_.active = false;
				}
			}
			break;
		case Envelope::Direction::Amplify:
			if (envelope_.current_volume < 0x0F)
			{
				if (++envelope_.current_volume == 0x0F)
				{
					envelope_.active = false;
				}
			}
			break;
		default:
			throw std::logic_error{ "Invalid envelope direction: " + static_cast<size_t>(envelope_.direction) };
		}

		envelope_.cycles_left = envelope_.period;
	}
}

void NoiseChannel::OnNR41Written(uint8_t value)
{
	length_counter_ = 64 - (value & 0x3F);
	//TODO: is length counter enabled?
}

void NoiseChannel::OnNR42Written(uint8_t value)
{
	envelope_.period = value & 0x07;
	envelope_.cycles_left = envelope_.period;
	envelope_.direction = (value & 0x08) != 0 ? Envelope::Direction::Amplify : Envelope::Direction::Attenuate;
	envelope_.initial_volume = value >> 4;

	// Disable channel if DAC disabled?
	enabled_ = IsDacOn();
}

void NoiseChannel::OnNR43Written(uint8_t value)
{
	uint8_t frequency_divisors_[8]{ 8, 16, 32, 48, 64, 80, 96, 112 };
	frequency_divisor_ = frequency_divisors_[value & 0x07];
	divisor_left_shift_ = value >> 4;
	seven_bits_lfsr_ = (value & 0x08) != 0;
	UpdateClockDividerPeriod();
}

void NoiseChannel::OnNR44Written(uint8_t value)
{
	length_counter_enabled_ = (value & 0x40) != 0;

	if ((value & 0x80) != 0)
	{
		Trigger();
	}
}

void NoiseChannel::Trigger()
{
	clock_divider_.Reset();
	lfsr_ = 0x7FFF;

	if (length_counter_ == 0)
	{
		length_counter_ = 64;
	}

	envelope_.cycles_left = envelope_.period;
	envelope_.current_volume = envelope_.initial_volume;
	envelope_.active = true;

	if (IsDacOn()) { enabled_ = true; }
}
