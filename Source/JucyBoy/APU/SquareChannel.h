#pragma once

#include <cstdint>
#include "ClockDivider.h"

class SquareChannel
{
public:
	SquareChannel() = default;
	virtual ~SquareChannel() {}

	// Clock divider
	void OnClockCyclesLapsed(size_t num_clock_cycles);
	void OnClockDividerTicked();

	// Interface with frame sequencer
	void ClockLengthCounter();
	void ClockVolumeEnvelope();

	// Interface with memory registers
	inline uint8_t ReadNRx1() const { return 0x3F | static_cast<uint8_t>(selected_duty_cycle_ << 6); }
	inline uint8_t ReadNRx2() const { return static_cast<uint8_t>(envelope_.period) | (static_cast<uint8_t>(envelope_.direction) << 3) | static_cast<uint8_t>(envelope_.initial_volume << 4); }
	inline uint8_t ReadNRx3() const { return 0xFF; }
	inline uint8_t ReadNRx4() const { return 0xBF | (length_counter_enabled_ << 6); }
	void OnNRx1Written(uint8_t value);
	void OnNRx2Written(uint8_t value);
	void OnNRx3Written(uint8_t value);
	void OnNRx4Written(uint8_t value);

	size_t GetSample() const;
	bool IsChannelOn() const { return enabled_; }
	void Reset();
	void Disable() { enabled_ = false; }

	template<class Archive>
	void serialize(Archive &archive);

protected:
	virtual void Trigger();
	inline void UpdateClockDividerPeriod() { clock_divider_.SetPeriod((2048 - frequency_) * 4); }

private:
	inline bool IsDacOn() const { return (envelope_.initial_volume != 0) || (envelope_.direction == Envelope::Direction::Amplify); }

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

	ClockDivider clock_divider_{ (2048 - frequency_) * 4, std::bind(&SquareChannel::OnClockDividerTicked, this) };

	static constexpr uint8_t duty_cycles_[4]{ 0x01, 0x81, 0x87, 0x7E };
};

struct SquareChannelWithSweep final : public SquareChannel
{
public:
	SquareChannelWithSweep();
	~SquareChannelWithSweep() = default;

	// Interface with frame sequencer
	void ClockFrequencySweep();

	// Interface with memory registers
	uint8_t ReadNR10() const { return 0x80 | static_cast<uint8_t>(frequency_sweep_.shift) | static_cast<uint8_t>(frequency_sweep_.direction << 3) | static_cast<uint8_t>(frequency_sweep_.period << 4); }
	void OnNR10Written(uint8_t value);

	template<class Archive>
	void serialize(Archive &archive);

private:
	void Trigger() override;
	inline size_t CalculateNewFrequency() const { return frequency_sweep_.current_frequency + (frequency_sweep_.direction * (frequency_sweep_.current_frequency >> frequency_sweep_.shift)); }

private:
	struct FrequencySweep
	{
		size_t period{ 0 };
		int direction{ 0 };
		size_t shift{ 0 };

		bool active{ false };
		size_t current_frequency{ 0 };
		size_t cycles_left{ 0 };
	} frequency_sweep_;
};

template<class Archive>
void SquareChannel::serialize(Archive &archive)
{
	archive(enabled_, frequency_, selected_duty_cycle_, duty_cycle_step_, length_counter_, length_counter_enabled_);
	archive(envelope_.initial_volume, envelope_.direction, envelope_.period, envelope_.active, envelope_.current_volume, envelope_.cycles_left);
	archive(clock_divider_);
}

template<class Archive>
void SquareChannelWithSweep::serialize(Archive &archive)
{
	archive(static_cast<SquareChannel&>(*this));
	archive(frequency_sweep_.period, frequency_sweep_.direction, frequency_sweep_.shift, frequency_sweep_.active, frequency_sweep_.current_frequency, frequency_sweep_.cycles_left);
}
