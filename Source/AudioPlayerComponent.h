#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <array>

class APU;

class AudioPlayerComponent final : public juce::AudioAppComponent
{
public:
	AudioPlayerComponent(APU &apu);
	~AudioPlayerComponent();

	void ClearBuffer();

	void paint(Graphics&) override {}
	void resized() override {}

	// juce::AudioAppComponent overrides
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void releaseResources() override;
	void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override;

	// APU Listener functions
	void OnNewSample(size_t right_sample, size_t left_sample);

private:
	struct OutputBuffer
	{
		std::array<size_t, 1024> samples{};
		juce::AbstractFifo abstract_fifo{ static_cast<int>(samples.size()) };
	};
	
	std::array<OutputBuffer, 2> output_buffers_;

	// Downsampling
	size_t output_sample_rate_{ 0 };
	size_t downsampling_ratio_integer_part_{ 0 };
	size_t downsampling_ratio_remainder_{ 0 };

	std::array<size_t, 2> input_sample_accumulators_{};
	size_t num_accumulated_apu_samples_{ 0 };
	size_t num_apu_samples_in_next_output_sample_{ 0 };

	APU* apu_{ nullptr };

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};
