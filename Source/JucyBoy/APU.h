#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <array>
#include "Memory.h"
#include "APU/SquareChannel.h"
#include "APU/WaveChannel.h"
#include "APU/NoiseChannel.h"
#include "CPU.h"

class APU final
{
	static constexpr size_t max_channel_volume_{ 15 };
	static constexpr size_t max_master_volume_{ 8 };
	static constexpr size_t input_clock_frequency_{ 4194304 };
	static constexpr size_t frame_sequencer_frequency_{ 512 };

public:
	static constexpr size_t num_channels_{ 4 };
	static constexpr size_t num_outputs_{ 2 };
	enum Outputs
	{
		Left = 0,
		Right = 1
	};
	static constexpr size_t sample_rate_{ input_clock_frequency_ / 2 };
	static constexpr size_t max_amplitude_{ max_master_volume_ * num_channels_ * max_channel_volume_ };

	APU() = default;
	~APU() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse();

	// Frame Sequencer callback function
	void OnFrameSequencerClocked();

	// MMU mapped memory read/write functions
	uint8_t OnIoMemoryRead(Memory::Address address) const;
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	// AddListener returns a deregister function that can be called with no arguments
	using SampleBatch = std::array<std::array<size_t, num_channels_>, num_outputs_>;
	using Listener = std::function<void(SampleBatch &sample_batch)>;
	std::function<void()> AddListener(Listener listener);

	template<class Archive>
	void serialize(Archive &archive);

private:
	void ClockLengthCounters();

	// Listener notification
	void NotifyNewSample(SampleBatch &sample_batch);

private:
	ClockDivider frame_sequencer_divider_{ input_clock_frequency_ / frame_sequencer_frequency_, std::bind(&APU::OnFrameSequencerClocked, this) };
	size_t frame_sequencer_step_{ 0 };

	SquareChannelWithSweep channel_1_;
	SquareChannel channel_2_;
	WaveChannel channel_3_;
	NoiseChannel channel_4_;

	// Control
	bool apu_enabled_{ true };
	std::array<size_t, num_outputs_> master_volumes_{7, 7};
	std::array<uint8_t, num_outputs_> channels_enabled_{0x3, 0xF};

	std::list<Listener> listeners_;
};

template<class Archive>
void APU::serialize(Archive &archive)
{
	archive(frame_sequencer_divider_, frame_sequencer_step_, channel_1_, channel_2_, apu_enabled_, master_volumes_, channels_enabled_);
}
