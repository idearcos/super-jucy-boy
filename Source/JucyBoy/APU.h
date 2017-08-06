#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include "Memory.h"
#include "SquareWaveChannel.h"
#include "CPU.h"

class APU final : public CPU::Listener
{
	static constexpr size_t num_channels_{ 4 };
	static constexpr size_t max_channel_volume_{ 15 };
	static constexpr size_t max_master_volume_{ 8 };
	static constexpr size_t input_clock_frequency_{ 4194304 };
	static constexpr size_t frame_sequencer_frequency_{ 512 };

public:
	static constexpr size_t sample_rate_{ input_clock_frequency_ / 4 };
	static constexpr size_t max_amplitude_{ max_master_volume_ * num_channels_ * max_channel_volume_ };

	APU() = default;
	~APU() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse() override;

	// Frame Sequencer callback function
	void OnFrameSequencerClocked();

	// MMU mapped memory read/write functions
	uint8_t OnIoMemoryRead(const Memory::Address &address) const;
	void OnIoMemoryWritten(const Memory::Address &address, uint8_t value);

	// AddListener returns a deregister function that can be called with no arguments
	using Listener = std::function<void(size_t right_sample, size_t left_sample)>;
	std::function<void()> AddListener(Listener listener);

	template<class Archive>
	void serialize(Archive &archive);

private:
	void ClockLengthCounters();

	// Listener notification
	void NotifyNewSample(size_t right_sample, size_t left_sample);

private:
	ClockDivider frame_sequencer_divider_{ input_clock_frequency_ / frame_sequencer_frequency_, std::bind(&APU::OnFrameSequencerClocked, this) };
	size_t frame_sequencer_step_{ 0 };

	SquareWaveChannelWithSweep channel_1_;
	SquareWaveChannel channel_2_;

	// Control
	bool apu_enabled_{ true };
	size_t right_volume_{ 7 };
	size_t left_volume_{ 7 };
	uint8_t right_channels_enabled_{ 0x3 };
	uint8_t left_channels_enabled_{ 0xF};

	std::list<Listener> listeners_;
};

template<class Archive>
void APU::serialize(Archive &archive)
{
	archive(frame_sequencer_divider_, frame_sequencer_step_, channel_1_, channel_2_, apu_enabled_, right_volume_, left_volume_, right_channels_enabled_, left_channels_enabled_);
}
