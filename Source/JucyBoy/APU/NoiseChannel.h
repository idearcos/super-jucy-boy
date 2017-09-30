#pragma once

#include <cstdint>
#include <algorithm>
#include "ClockDivider.h"

class NoiseChannel
{
public:
	NoiseChannel() = default;
	virtual ~NoiseChannel() {}

	// Clock divider
	void OnClockCyclesLapsed(size_t num_clock_cycles);
	void OnClockDividerTicked();

	// Interface with frame sequencer
	void ClockLengthCounter();
	void ClockVolumeEnvelope();

	// Interface with memory registers
	inline uint8_t ReadNR41() const { return 0xFF; }
	inline uint8_t ReadNR42() const { return envelope_.period | (static_cast<uint8_t>(envelope_.direction) << 3) | (envelope_.initial_volume << 4); }
	inline uint8_t ReadNR43() const { return static_cast<uint8_t>((std::min(frequency_divisor_ >> 4, 1)) | (seven_bits_lfsr_ << 3) | (divisor_left_shift_ << 4)); }
	inline uint8_t ReadNR44() const { return 0xBF | (length_counter_enabled_ << 6); }
	void OnNR41Written(uint8_t value);
	void OnNR42Written(uint8_t value);
	void OnNR43Written(uint8_t value);
	void OnNR44Written(uint8_t value);

	size_t GetSample() const;
	bool IsChannelOn() const { return enabled_; }
	void Reset();
	void Disable() { enabled_ = false; }

protected:
	virtual void Trigger();
	inline void UpdateClockDividerPeriod() { clock_divider_.SetPeriod(frequency_divisor_ << divisor_left_shift_); }

private:
	inline bool IsDacOn() const { return (envelope_.initial_volume != 0) || (envelope_.direction == Envelope::Direction::Amplify); }

private:
	bool enabled_{ false };

	size_t length_counter_{ 0 };
	bool length_counter_enabled_{ false };

	size_t lfsr_{ 0x7FFF };
	uint8_t frequency_divisor_{ 8 };
	uint8_t divisor_left_shift_{ 0 };
	bool seven_bits_lfsr_{ false };

	struct Envelope
	{
		uint8_t initial_volume{ 0 };
		enum class Direction
		{
			Attenuate,
			Amplify
		} direction{ Direction::Attenuate };
		uint8_t period{ 0 };

		bool active{ false };
		uint8_t current_volume{ 0 };
		uint8_t cycles_left{ 0 };
	} envelope_;

	ClockDivider clock_divider_{ static_cast<size_t>(frequency_divisor_ << divisor_left_shift_), std::bind(&NoiseChannel::OnClockDividerTicked, this) };
};
