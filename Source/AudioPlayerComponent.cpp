#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioPlayerComponent.h"
#include <numeric>

AudioPlayerComponent::AudioPlayerComponent()
{
	setAudioChannels(0, 2);
}

AudioPlayerComponent::~AudioPlayerComponent()
{
	shutdownAudio();
}

void AudioPlayerComponent::ClearBuffer()
{
	abstract_fifo_.reset();
}

void AudioPlayerComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	// Due to how Juce's AbstractFifo works internally, the size needs to be set to 1 element bigger than intended
	abstract_fifo_.setTotalSize(samplesPerBlockExpected + 1);
	for (auto &output_channels : output_buffers_)
	{
		for (auto &channel_buffer : output_channels)
		{
			// The buffers themselves need to be 1 element bigger as well
			channel_buffer.resize(samplesPerBlockExpected + 1);
		}
	}

	output_sample_rate_ = static_cast<size_t>(sampleRate);
	downsampling_ratio_integer_part_ = APU::sample_rate_ / output_sample_rate_;
	downsampling_ratio_remainder_ = APU::sample_rate_ % output_sample_rate_;
}

void AudioPlayerComponent::releaseResources()
{

}

void AudioPlayerComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
	bufferToFill.clearActiveBufferRegion();

	if (abstract_fifo_.getNumReady() < bufferToFill.numSamples)
	{
		return;
	}

	int startIndexes[2]{ 0, 0 }, blockSizes[2]{ 0, 0 };
	abstract_fifo_.prepareToRead(bufferToFill.numSamples, startIndexes[0], blockSizes[0], startIndexes[1], blockSizes[1]);
	int writeOffsets[2]{ 0, blockSizes[0] };

	// Convert APU amplitude from [0, APU::max_amplitude_] to [0, +0.50], then to [-0.25, +0.25]
	constexpr auto max_amplitude = 0.50f;
	constexpr auto half_max_amplitude = max_amplitude / 2.0f;
	constexpr auto amplitude_divisor = APU::max_amplitude_ / max_amplitude;

	//TODO: handle case of mono by mixing both outputs

	for (int output_channel = 0; output_channel < bufferToFill.buffer->getNumChannels(); ++output_channel)
	{
		const auto output_index = output_channel % APU::num_outputs_;

		float* const buffer = bufferToFill.buffer->getWritePointer(output_channel, bufferToFill.startSample);

		// 2 blocks of data in the circular buffer
		for (int block_index = 0; block_index < 2; ++block_index)
		{
			// Iterate through all 4 channels
			for (int channel_index = 0; channel_index < APU::num_channels_; ++channel_index)
			{
				// If the channel has been disabled by the GUI, proceed to next
				if (!channels_enabled_[channel_index]) continue;

				// Contribute to all samples of this block
				for (int sample_index = 0; sample_index < blockSizes[block_index]; ++sample_index)
				{
					buffer[writeOffsets[block_index] + sample_index] += output_buffers_[output_index][channel_index][startIndexes[block_index] + sample_index];
				}
			}

			// Once all channels have contributed, convert the amplitude of samples to the range of [-0.25, +0.25]
			for (int sample_index = 0; sample_index < blockSizes[block_index]; ++sample_index)
			{
				buffer[writeOffsets[block_index] + sample_index] /= amplitude_divisor;
				buffer[writeOffsets[block_index] + sample_index] -= half_max_amplitude;
			}
		}
	}

	abstract_fifo_.finishedRead(bufferToFill.numSamples);
}

void AudioPlayerComponent::OnNewSamples(APU::SampleBatch &sample_batch)
{
	for (int output_index = 0; output_index < APU::num_outputs_; ++output_index)
	{
		for (int channel_index = 0; channel_index < APU::num_channels_; ++channel_index)
		{
			input_sample_accumulators_[output_index][channel_index] += sample_batch[output_index][channel_index];
		}
	}

	if (++num_accumulated_apu_samples_ < downsampling_ratio_integer_part_) return;

	while (abstract_fifo_.getFreeSpace() == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
	}

	int startIndex1{ 0 }, blockSize1{ 0 }, startIndex2{ 0 }, blockSize2{ 0 };
	abstract_fifo_.prepareToWrite(1, startIndex1, blockSize1, startIndex2, blockSize2);

	const auto sample_interpolation_ratio = static_cast<float>(downsampling_ratio_remainder_) / output_sample_rate_;

	// Calculate the average of accumulated samples and push the values to the output buffers
	for (int output_index = 0; output_index < APU::num_outputs_; ++output_index)
	{
		for (int channel_index = 0; channel_index < APU::num_channels_; ++channel_index)
		{
			// Interpolate between previous and current accumulated values
			output_buffers_[output_index][channel_index][startIndex1] = (sample_interpolation_ratio * previous_accumulator_values_[output_index][channel_index]
				+ (1.0f - sample_interpolation_ratio) * input_sample_accumulators_[output_index][channel_index])
				/ num_accumulated_apu_samples_;

			previous_accumulator_values_[output_index][channel_index] = input_sample_accumulators_[output_index][channel_index];

			// Clear input samples accumulators
			input_sample_accumulators_[output_index][channel_index] = 0;
		}
	}

	num_accumulated_apu_samples_ = 0;

	downsampling_ratio_remainder_ += APU::sample_rate_ % output_sample_rate_;
	if (downsampling_ratio_remainder_ > output_sample_rate_)
	{
		downsampling_ratio_remainder_ -= output_sample_rate_;
	}

	abstract_fifo_.finishedWrite(1);
}
