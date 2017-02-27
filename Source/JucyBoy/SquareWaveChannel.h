#pragma once

#include <cstdint>
#include "ClockDivider.h"

class SquareWaveChannel
{
public:
	SquareWaveChannel() = default;
	virtual ~SquareWaveChannel() {}

	// Clock divider
	void OnMachineCycleLapse();
	void OnClockDividerTicked();

	// Interface with frame sequencer
	void ClockLengthCounter();
	void ClockVolumeEnvelope();

	// Interface with memory registers
	void OnNRx1Written(uint8_t value);
	void OnNRx2Written(uint8_t value);
	void OnNRx3Written(uint8_t value);
	void OnNRx4Written(uint8_t value);

	size_t GetSample() const;
	bool IsChannelOn() const { return enabled_; }
	void Reset();

protected:
	virtual void Trigger();
	inline void UpdateClockDividerPeriod() { clock_divider_.SetPeriod((2048 - frequency_) * 4); }

private:
	inline bool IsDacEnabled() const { return (envelope_.initial_volume != 0) || (envelope_.direction == Envelope::Direction::Amplify); }

protected:
	bool enabled_{ false };
	size_t frequency_{ 0x7FF };

private:
	size_t selected_duty_cycle_{ 0 };
	size_t duty_cycle_step_{ 0 };

	size_t length_counter_{ 0 };
	bool length_counter_enabled_{ false };

	struct Envelope
	{
		size_t initial_volume{ 0 };
		enum class Direction
		{
			Attenuate,
			Amplify
		} direction{ Direction::Attenuate };
		size_t period{ 0 };

		bool active{ false };
		size_t current_volume{ 0 };
		size_t cycles_left{ 0 };
	} envelope_;

	ClockDivider clock_divider_{ (2048 - frequency_) * 4, std::bind(&SquareWaveChannel::OnClockDividerTicked, this) };

	static constexpr uint8_t duty_cycles_[4]{ 0x01, 0x81, 0x87, 0x7E };
};

struct SquareWaveChannelWithSweep final : public SquareWaveChannel
{
public:
	SquareWaveChannelWithSweep() : SquareWaveChannel() {}
	~SquareWaveChannelWithSweep() = default;

	// Interface with frame sequencer
	void ClockFrequencySweep();

	// Interface with memory registers
	void OnNR10Written(uint8_t value);

private:
	void Trigger() override;
	inline size_t CalculateNewFrequency() const { return frequency_sweep_.current_frequency + (frequency_sweep_.direction * (frequency_sweep_.current_frequency >> frequency_sweep_.shift)); }

private:
	struct FrequencySweep
	{
		size_t period{ 0 };
		int direction{ 1 };
		size_t shift{ 0 };

		bool active{ false };
		size_t current_frequency{ 0 };
		size_t cycles_left{ 0 };
	} frequency_sweep_;
};
