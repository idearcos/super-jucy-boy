#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/APU.h"
#include <cstdint>
#include <array>

class AudioPlayerComponent final : public juce::AudioAppComponent
{
public:
	AudioPlayerComponent();
	~AudioPlayerComponent();

	void ClearBuffer();

	void paint(juce::Graphics&) override {}
	void resized() override {}

	// juce::AudioAppComponent overrides
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void releaseResources() override;
	void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

	// APU Listener functions
	void OnNewSamples(APU::SampleBatch &sample_batch);

private:
	static const size_t output_buffer_size_{ 480 + 1 };
	using OutputBuffer = std::array<float, output_buffer_size_>;
	std::array<std::array<OutputBuffer, APU::num_channels_>, APU::num_outputs_> output_buffers_{};
	juce::AbstractFifo abstract_fifo_{ output_buffer_size_ };

	// Downsampling
	size_t output_sample_rate_{ 0 };
	size_t downsampling_ratio_integer_part_{ 0 };
	size_t downsampling_ratio_remainder_{ 0 };

	APU::SampleBatch input_sample_accumulators_{};
	size_t num_accumulated_apu_samples_{ 0 };

	APU::SampleBatch previous_accumulator_values_{};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};
