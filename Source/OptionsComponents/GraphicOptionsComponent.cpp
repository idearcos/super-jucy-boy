#include "../../JuceLibraryCode/JuceHeader.h"
#include "GraphicOptionsComponent.h"
#include "../GameScreenComponent.h"

GraphicOptionsComponent::GraphicOptionsComponent(GameScreenComponent &game_screen_component) :
	game_screen_component_{ &game_screen_component }
{
	addAndMakeVisible(nearest_mag_filter_toggle_);
	addAndMakeVisible(linear_mag_filter_toggle_);

	nearest_mag_filter_toggle_.setButtonText("Nearest neighbor");
	linear_mag_filter_toggle_.setButtonText("Linear");

	nearest_mag_filter_toggle_.setRadioGroupId(1);
	linear_mag_filter_toggle_.setRadioGroupId(1);

	nearest_mag_filter_toggle_.setToggleState(game_screen_component_->GetMagnificationFilter() == GL_NEAREST, juce::dontSendNotification);
	linear_mag_filter_toggle_.setToggleState(game_screen_component_->GetMagnificationFilter() == GL_LINEAR, juce::dontSendNotification);

	nearest_mag_filter_toggle_.addListener(this);
	linear_mag_filter_toggle_.addListener(this);
}

void GraphicOptionsComponent::buttonClicked(juce::Button* button)
{
	if ((button == &nearest_mag_filter_toggle_) && nearest_mag_filter_toggle_.getToggleState())
	{
		game_screen_component_->SetMagnificationFilter(GL_NEAREST);
	}
	else if ((button == &linear_mag_filter_toggle_) && linear_mag_filter_toggle_.getToggleState())
	{
		game_screen_component_->SetMagnificationFilter(GL_LINEAR);
	}
}


void GraphicOptionsComponent::paint(juce::Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void GraphicOptionsComponent::resized()
{
	auto working_area = getLocalBounds();
	const auto toggle_height = working_area.getHeight();
	nearest_mag_filter_toggle_.setBounds(working_area.removeFromTop(toggle_height / 2));
	linear_mag_filter_toggle_.setBounds(working_area.removeFromTop(toggle_height / 2));
}
