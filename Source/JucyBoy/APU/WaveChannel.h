#pragma once

#include <cstdint>
#include <array>
#include "ClockDivider.h"

class WaveChannel
{
public:
	WaveChannel() = default;
	virtual ~WaveChannel() {}

	// Clock divider
	void OnClockCyclesLapsed(size_t num_clock_cycles);
	void OnClockDividerTicked();

	// Interface with frame sequencer
	void ClockLengthCounter();

	// Interface with memory registers
	inline uint8_t ReadNR30() const { return 0x7F | (is_dac_on_ << 7); }
	inline uint8_t ReadNR31() const { return 0xFF; }
	inline uint8_t ReadNR32() const { return 0x9F | (((volume_right_shift_ == 4) ? 0 : (volume_right_shift_ + 1)) << 5); }
	inline uint8_t ReadNR33() const { return 0xFF; }
	inline uint8_t ReadNR34() const { return 0xBF | (length_counter_enabled_ << 6); }
	void OnNR30Written(uint8_t value);
	void OnNR31Written(uint8_t value);
	void OnNR32Written(uint8_t value);
	void OnNR33Written(uint8_t value);
	void OnNR34Written(uint8_t value);

	inline size_t GetWaveTableSample(size_t wave_table_index) const { return wave_table_[wave_table_index]; }
	inline void SetWaveTableSample(size_t wave_table_index, size_t value) { wave_table_[wave_table_index] = value; }
	size_t GetSample() const;
	bool IsChannelOn() const { return enabled_; }
	void Reset();
	void Disable() { enabled_ = false; }

protected:
	virtual void Trigger();
	inline void UpdateClockDividerPeriod() { clock_divider_.SetPeriod((2048 - frequency_) * 2); }

private:
	static const size_t wave_table_size_{ 32 };
	static const size_t max_sample_index_{ wave_table_size_ - 1 };

private:
	bool enabled_{ false };
	bool is_dac_on_{ false };

	std::array<size_t, wave_table_size_> wave_table_{};
	size_t current_sample_index_{ 0 };
	size_t sample_buffer_{ 0 };

	size_t frequency_{ 0x7FF };
	size_t volume_right_shift_{ 0 };

	size_t length_counter_{ 0 };
	bool length_counter_enabled_{ false };

	ClockDivider clock_divider_{ (2048 - frequency_) * 2, std::bind(&WaveChannel::OnClockDividerTicked, this) };
};
