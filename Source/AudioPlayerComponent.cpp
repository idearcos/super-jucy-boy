#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioPlayerComponent.h"
#include "JucyBoy/APU.h"
#include <cassert>

AudioPlayerComponent::AudioPlayerComponent(APU &apu) :
	apu_{ &apu }
{
	setAudioChannels(0, 2);
}

AudioPlayerComponent::~AudioPlayerComponent()
{
	shutdownAudio();
}

void AudioPlayerComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
	apu_->SetExpectedSampleRate(sampleRate / samplesPerBlockExpected, samplesPerBlockExpected);
}

void AudioPlayerComponent::releaseResources()
{

}

void AudioPlayerComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
{
	std::unique_lock<std::mutex> lock{ sample_blocks_mutex_ };
	if (right_channel_sample_blocks_.empty() || left_channel_sample_blocks_.empty())
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}
	
	const auto right_sample_block = right_channel_sample_blocks_.front();
	right_channel_sample_blocks_.pop();
	const auto left_sample_block = left_channel_sample_blocks_.front();
	left_channel_sample_blocks_.pop();

	lock.unlock();

	assert(right_sample_block.size() == bufferToFill.numSamples);
	assert(left_sample_block.size() == bufferToFill.numSamples);

	// Volume decreasing:
	// Max APU sample volume = 0x0F = 15
	// Decrease amplitude to the -0.125 - 0.125 range: A = A / 15 * 0.25 - 0.125;

	switch (bufferToFill.buffer->getNumChannels())
	{
	case 0:
		break;
	case 1:
		{float* const buffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			buffer[sample] = static_cast<float>(right_sample_block[sample]) / 60.0 - 0.125;
		}}
		break;
	case 2:
		{float* const buffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
		for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			buffer[sample] = static_cast<float>(right_sample_block[sample]) / 60.0 - 0.125;
		}}
		{float* const buffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
		for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			buffer[sample] = static_cast<float>(left_sample_block[sample]) / 60.0 - 0.125;
		}}
		break;
	}
}

void AudioPlayerComponent::OnNewSampleBlock(const std::vector<uint8_t> &right, const std::vector<uint8_t> &left)
{
	std::unique_lock<std::mutex> lock{ sample_blocks_mutex_ };
	right_channel_sample_blocks_.emplace(right);
	left_channel_sample_blocks_.emplace(left);
}
