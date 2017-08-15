#include "../JuceLibraryCode/JuceHeader.h"
#include "OptionsWindow.h"
#include "../AudioPlayerComponent.h"

AudioOptionsComponent::AudioOptionsComponent(AudioPlayerComponent &audio_player_component) :
	audio_player_component_{ &audio_player_component }
{
	addAndMakeVisible(toggle_audio_channel_1_);
	addAndMakeVisible(toggle_audio_channel_2_);
	addAndMakeVisible(toggle_audio_channel_3_);
	addAndMakeVisible(toggle_audio_channel_4_);

	toggle_audio_channel_1_.setButtonText("Channel 1");
	toggle_audio_channel_2_.setButtonText("Channel 2");
	toggle_audio_channel_3_.setButtonText("Channel 3");
	toggle_audio_channel_4_.setButtonText("Channel 4");

	toggle_audio_channel_1_.setToggleState(audio_player_component_->IsChannelEnabled<0>(), juce::NotificationType::dontSendNotification);
	toggle_audio_channel_2_.setToggleState(audio_player_component_->IsChannelEnabled<1>(), juce::NotificationType::dontSendNotification);
	toggle_audio_channel_3_.setToggleState(audio_player_component_->IsChannelEnabled<2>(), juce::NotificationType::dontSendNotification);
	toggle_audio_channel_4_.setToggleState(audio_player_component_->IsChannelEnabled<3>(), juce::NotificationType::dontSendNotification);

	toggle_audio_channel_1_.addListener(this);
	toggle_audio_channel_2_.addListener(this);
	toggle_audio_channel_3_.addListener(this);
	toggle_audio_channel_4_.addListener(this);
}

void AudioOptionsComponent::buttonClicked(juce::Button* button)
{
	if (button == &toggle_audio_channel_1_)
	{
		audio_player_component_->EnableChannel<0>(toggle_audio_channel_1_.getToggleState());
	}
	else if (button == &toggle_audio_channel_2_)
	{
		audio_player_component_->EnableChannel<1>(toggle_audio_channel_2_.getToggleState());
	}
	else if (button == &toggle_audio_channel_3_)
	{
		audio_player_component_->EnableChannel<2>(toggle_audio_channel_3_.getToggleState());
	}
	else if (button == &toggle_audio_channel_4_)
	{
		audio_player_component_->EnableChannel<3>(toggle_audio_channel_4_.getToggleState());
	}
}

void AudioOptionsComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);
}

void AudioOptionsComponent::resized()
{
	auto working_area = getLocalBounds();
	auto top_half = working_area.removeFromTop(working_area.getHeight() / 2);
	toggle_audio_channel_1_.setBounds(top_half.removeFromLeft(top_half.getWidth() / 2));
	toggle_audio_channel_2_.setBounds(top_half);
	toggle_audio_channel_3_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	toggle_audio_channel_4_.setBounds(working_area);
}
