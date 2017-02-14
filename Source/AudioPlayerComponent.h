#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>

class APU;

class AudioPlayerComponent final : public juce::AudioAppComponent
{
public:
	AudioPlayerComponent(APU &apu);
	~AudioPlayerComponent();

	void paint(Graphics&) override {}
	void resized() override {}

	// juce::AudioAppComponent overrides
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void releaseResources() override;
	void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override;

	// APU Listener functions
	void OnNewSampleBlock(const std::vector<uint8_t> &right, const std::vector<uint8_t> &left);

private:
	std::queue<std::vector<uint8_t>> right_channel_sample_blocks_;
	std::queue<std::vector<uint8_t>> left_channel_sample_blocks_;
	std::mutex sample_blocks_mutex_;
	
	APU* apu_{ nullptr };

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlayerComponent)
};
