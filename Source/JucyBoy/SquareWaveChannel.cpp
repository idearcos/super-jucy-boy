#include "SquareWaveChannel.h"
#include <stdexcept>

size_t SquareWaveChannel::GetSample() const
{
	return enabled_ * ((duty_cycles_[selected_duty_cycle_] >> duty_cycle_step_) & 0x01) * envelope_.current_volume;
}

void SquareWaveChannel::Reset()
{
	clock_divider_.Reset();
	duty_cycle_step_ = 0;
}

void SquareWaveChannel::OnMachineCycleLapse()
{
	if (!enabled_) return;

	clock_divider_.OnInputClockCyclesLapsed(4);
}

void SquareWaveChannel::OnClockDividerTicked()
{
	duty_cycle_step_ = (duty_cycle_step_ + 1) & 0x07;
}

void SquareWaveChannel::ClockLengthCounter()
{
	if (length_counter_enabled_ && (length_counter_ > 0))
	{
		if (--length_counter_ == 0)
		{
			enabled_ = false;
		}
	}
}

void SquareWaveChannel::ClockVolumeEnvelope()
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

void SquareWaveChannel::OnNRx1Written(uint8_t value)
{
	length_counter_ = 64 - (value & 0x3F);
	selected_duty_cycle_ = value >> 6;
	//TODO: is length counter enabled?
}

void SquareWaveChannel::OnNRx2Written(uint8_t value)
{
	envelope_.period = value & 0x07;
	envelope_.cycles_left = envelope_.period;
	envelope_.direction = (value & 0x08) != 0 ? Envelope::Direction::Amplify : Envelope::Direction::Attenuate;
	envelope_.initial_volume = value >> 4;

	// Disable channel if DAC disabled?
	enabled_ = IsDacEnabled();
}

void SquareWaveChannel::OnNRx3Written(uint8_t value)
{
	frequency_ = (frequency_ & 0x700) | value;
	UpdateClockDividerPeriod();
}

void SquareWaveChannel::OnNRx4Written(uint8_t value)
{
	frequency_ = (frequency_ & 0xFF) | ((value & 0x07) << 8);
	UpdateClockDividerPeriod();

	length_counter_enabled_ = (value & 0x40) != 0;

	if ((value & 0x80) != 0)
	{
		Trigger();
	}
}

void SquareWaveChannel::Trigger()
{
	clock_divider_.Reset();

	if (length_counter_ == 0)
	{
		length_counter_ = 64;
	}

	envelope_.cycles_left = envelope_.period;
	envelope_.current_volume = envelope_.initial_volume;
	envelope_.active = true;

	if (IsDacEnabled()) { enabled_ = true; }
}

SquareWaveChannelWithSweep::SquareWaveChannelWithSweep() :
	SquareWaveChannel()
{
	OnNR10Written(0x80);
	OnNRx1Written(0xBF);
	OnNRx2Written(0xF3);
	OnNRx4Written(0xBF);
}

void SquareWaveChannelWithSweep::ClockFrequencySweep()
{
	if (!enabled_) return;
	if (!frequency_sweep_.active) return;
	if (frequency_sweep_.period == 0) return;

	if (--frequency_sweep_.cycles_left > 0) return;
	frequency_sweep_.cycles_left = frequency_sweep_.period;

	const auto new_frequency = CalculateNewFrequency();

	if (new_frequency > 2047)
	{
		enabled_ = false;
		return;
	}

	frequency_sweep_.current_frequency = new_frequency;
	frequency_ = new_frequency;
	UpdateClockDividerPeriod();

	const auto new_frequency_2 = CalculateNewFrequency();
	if (new_frequency_2 > 2047)
	{
		enabled_ = false;
	}
}

void SquareWaveChannelWithSweep::OnNR10Written(uint8_t value)
{
	frequency_sweep_.shift = value & 0x07;
	frequency_sweep_.direction = ((value & 0x08) != 0) ? -1 : 1;
	frequency_sweep_.period = (value & 0x70) >> 4;
}

void SquareWaveChannelWithSweep::Trigger()
{
	SquareWaveChannel::Trigger();

	frequency_sweep_.current_frequency = frequency_;
	frequency_sweep_.cycles_left = 0;
	frequency_sweep_.active = (frequency_sweep_.period != 0) || (frequency_sweep_.shift != 0);
}
