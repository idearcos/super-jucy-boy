#include "WaveChannel.h"
#include <stdexcept>

size_t WaveChannel::GetSample() const
{
	return (enabled_ * sample_buffer_) >> volume_right_shift_;
}

void WaveChannel::Reset()
{
	clock_divider_.Reset();
	current_sample_index_ = 0;
}

void WaveChannel::OnClockCyclesLapsed(size_t num_clock_cycles)
{
	if (!enabled_) return;

	clock_divider_.OnInputClockCyclesLapsed(num_clock_cycles);
}

void WaveChannel::OnClockDividerTicked()
{
	current_sample_index_ = (current_sample_index_ + 1) & max_sample_index_;
	sample_buffer_ = wave_table_[current_sample_index_];
}

void WaveChannel::ClockLengthCounter()
{
	if (length_counter_enabled_ && (length_counter_ > 0))
	{
		if (--length_counter_ == 0)
		{
			enabled_ = false;
		}
	}
}

void WaveChannel::OnNR30Written(uint8_t value)
{
	is_dac_on_ = ((value & 0x80) != 0);
	if (!is_dac_on_) enabled_ = false;
}

void WaveChannel::OnNR31Written(uint8_t value)
{
	length_counter_ = 256 - value;
	//TODO: is length counter enabled?
}

void WaveChannel::OnNR32Written(uint8_t value)
{
	const auto volume_code = static_cast<uint8_t>((value >> 5) & 0x03);
	volume_right_shift_ = (volume_code != 0) ? (volume_code - 1) : 4;
}

void WaveChannel::OnNR33Written(uint8_t value)
{
	frequency_ = (frequency_ & 0x700) | value;
	UpdateClockDividerPeriod();
}

void WaveChannel::OnNR34Written(uint8_t value)
{
	frequency_ = (frequency_ & 0xFF) | ((value & 0x07) << 8);
	UpdateClockDividerPeriod();

	length_counter_enabled_ = (value & 0x40) != 0;

	if ((value & 0x80) != 0)
	{
		Trigger();
	}
}

void WaveChannel::Trigger()
{
	clock_divider_.Reset();

	if (length_counter_ == 0)
	{
		length_counter_ = 256;
	}

	// Sample buffer is NOT refilled, but the position is updated
	current_sample_index_ = 0;

	if (is_dac_on_) enabled_ = true;
}
