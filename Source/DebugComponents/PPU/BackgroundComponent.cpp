#include "BackgroundComponent.h"

BackgroundComponent::BackgroundComponent(BackgroundRenderer &background_renderer) :
	background_renderer_{ &background_renderer }
{
	addAndMakeVisible(select_tile_map_auto_);
	addAndMakeVisible(select_tile_map_0_);
	addAndMakeVisible(select_tile_map_1_);

	select_tile_map_auto_.setButtonText("Auto");
	select_tile_map_0_.setButtonText("#0");
	select_tile_map_1_.setButtonText("#1");

	select_tile_map_auto_.setRadioGroupId(1);
	select_tile_map_0_.setRadioGroupId(1);
	select_tile_map_1_.setRadioGroupId(1);

	const auto selected_tile_map = background_renderer_->GetSelectedTileMap();
	select_tile_map_auto_.setToggleState(!selected_tile_map.has_value(), juce::dontSendNotification);
	select_tile_map_0_.setToggleState(selected_tile_map.has_value() && (*selected_tile_map == 0), juce::dontSendNotification);
	select_tile_map_1_.setToggleState(selected_tile_map.has_value() && (*selected_tile_map == 1), juce::dontSendNotification);

	select_tile_map_auto_.addListener(this);
	select_tile_map_0_.addListener(this);
	select_tile_map_1_.addListener(this);
}

BackgroundComponent::~BackgroundComponent()
{
}

void BackgroundComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void BackgroundComponent::resized()
{
	auto working_area = getLocalBounds();
	select_tile_map_auto_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 3));
	select_tile_map_0_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	select_tile_map_1_.setBounds(working_area);
}

void BackgroundComponent::buttonClicked(juce::Button* button)
{
	if ((button == &select_tile_map_auto_) && select_tile_map_auto_.getToggleState())
	{
		background_renderer_->SetSelectedTileMap({});
	}
	else if ((button == &select_tile_map_0_) && select_tile_map_0_.getToggleState())
	{
		background_renderer_->SetSelectedTileMap(0);
	}
	else if ((button == &select_tile_map_1_) && select_tile_map_1_.getToggleState())
	{
		background_renderer_->SetSelectedTileMap(1);
	}
}
