#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioPlayerComponent.h"
#include "JucyBoy/APU.h"


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
	for (auto &output_buffer : output_buffers_)
	{
		output_buffer.abstract_fifo.reset();
	}
}

void AudioPlayerComponent::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
	output_sample_rate_ = static_cast<size_t>(sampleRate);
	downsampling_ratio_integer_part_ = APU::sample_rate_ / output_sample_rate_;
	downsampling_ratio_remainder_ = APU::sample_rate_ % output_sample_rate_;

	num_apu_samples_in_next_output_sample_ = downsampling_ratio_integer_part_;
}

void AudioPlayerComponent::releaseResources()
{

}

void AudioPlayerComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
{
	if (output_buffers_[0].abstract_fifo.getNumReady() < bufferToFill.numSamples || output_buffers_[1].abstract_fifo.getNumReady()  < bufferToFill.numSamples)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	// Convert APU amplitude from [0, APU::max_amplitude_] to [0, +0.50], then to [-0.25, +0.25]
	const auto amplitude_divisor = APU::max_amplitude_ * 2.0f;

	for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
	{
		const auto output_index = channel % output_buffers_.size();
		int startIndex1{ 0 }, blockSize1{ 0 }, startIndex2{ 0 }, blockSize2{ 0 };
		output_buffers_[output_index].abstract_fifo.prepareToRead(bufferToFill.numSamples, startIndex1, blockSize1, startIndex2, blockSize2);

		float* const buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

		for (int sample = 0; sample < blockSize1; ++sample)
		{
			buffer[sample] = output_buffers_[output_index].samples[startIndex1 + sample] / amplitude_divisor - 0.25f;
		}
		for (int sample = 0; sample < blockSize2; ++sample)
		{
			buffer[blockSize1 + sample] = output_buffers_[output_index].samples[startIndex2 + sample] / amplitude_divisor - 0.25f;
		}
	}

	for (auto &output_buffer : output_buffers_)
	{
		output_buffer.abstract_fifo.finishedRead(bufferToFill.numSamples);
	}
}

void AudioPlayerComponent::OnNewSample(size_t right_sample, size_t left_sample)
{
	input_sample_accumulators_[0] += left_sample;
	input_sample_accumulators_[1] += right_sample;

	if (++num_accumulated_apu_samples_ < num_apu_samples_in_next_output_sample_) return;

	while (output_buffers_[0].abstract_fifo.getFreeSpace() == 0 || output_buffers_[0].abstract_fifo.getFreeSpace() == 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds{ 15 });
	}

	for (int i = 0; i < output_buffers_.size(); ++i)
	{
		int startIndex1{ 0 }, blockSize1{ 0 }, startIndex2{ 0 }, blockSize2{ 0 };
		output_buffers_[i].abstract_fifo.prepareToWrite(1, startIndex1, blockSize1, startIndex2, blockSize2);
		output_buffers_[i].samples[startIndex1] = input_sample_accumulators_[i] / num_accumulated_apu_samples_;
	}

	input_sample_accumulators_[0] = 0;
	input_sample_accumulators_[1] = 0;
	num_accumulated_apu_samples_ = 0;

	// Calculate how many APU samples will be used for the next output sample
	num_apu_samples_in_next_output_sample_ = downsampling_ratio_integer_part_;
	downsampling_ratio_remainder_ += APU::sample_rate_ % output_sample_rate_;
	if (downsampling_ratio_remainder_ > output_sample_rate_)
	{
		num_apu_samples_in_next_output_sample_ += 1;
		downsampling_ratio_remainder_ -= output_sample_rate_;
	}

	for (auto &output_buffer : output_buffers_)
	{
		output_buffer.abstract_fifo.finishedWrite(1);
	}
}
