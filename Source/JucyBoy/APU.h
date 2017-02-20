#pragma once

#include <cstdint>
#include <bitset>
#include <functional>
#include <vector>
#include <list>
#include "Memory.h"
#include "SquareWaveChannel.h"
#include "CPU.h"

class MMU;

class APU final : public CPU::Listener
{
public:
	APU(MMU &mmu);
	~APU() = default;

	// CPU::Listener overrides
	void OnMachineCycleLapse() override;

	// Frame Sequencer callback function
	void OnFrameSequencerClocked();

	// MMU listener functions
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	void SetExpectedSampleRate(size_t sample_blocks_per_second, size_t expected_samples_per_block);

	// AddListener returns a deregister function that can be called with no arguments
	using Listener = std::function<void(const std::vector<uint8_t> &right, const std::vector<uint8_t> &left)>;
	std::function<void()> AddListener(Listener listener);

private:
	void ClockLengthCounters();

	// Listener notification
	void NotifyNewSampleBlock();

private:
	ClockDivider frame_sequencer_divider_{ 4194304 / 512, std::bind(&APU::OnFrameSequencerClocked, this) };
	size_t frame_sequencer_step_{ 0 };

	SquareWaveChannelWithSweep channel_1_;
	SquareWaveChannel channel_2_;

	// Control
	bool apu_enabled_{ true };
	size_t right_volume_{ 0 };
	size_t left_volume_{ 0 };
	std::bitset<4> right_channels_enabled_{ 0 };
	std::bitset<4> left_channels_enabled_{ 0 };

	std::vector<uint8_t> right_channel_apu_samples_;
	std::vector<uint8_t> left_channel_apu_samples_;
	std::vector<uint8_t> right_channel_expected_samples_;
	std::vector<uint8_t> left_channel_expected_samples_;
	size_t num_samples_in_current_block_{ 0 };

	// Downsampling APU sample rate -> expected sample rate
	static constexpr size_t samples_per_frame_{ 70224 / 4 };
	static constexpr size_t frame_rate_{ 60 }; //TODO: might not always be the case
	static constexpr size_t samples_per_second_{ samples_per_frame_ * frame_rate_ };
	size_t sample_blocks_per_second_{ 0 };
	size_t apu_samples_per_block_integer_part_{ 0 };
	size_t apu_samples_in_next_block_{ 0 };
	size_t last_block_apu_samples_remainder_{ 0 };

	MMU* mmu_{ nullptr };

	std::list<Listener> listeners_;
};
