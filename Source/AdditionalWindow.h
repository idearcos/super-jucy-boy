#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AdditionalWindow : public juce::DocumentWindow
{
public:
	AdditionalWindow(juce::Component &content_component, const juce::String& window_title, juce::Colour background_colour, int required_buttons_) :
		juce::DocumentWindow{ window_title, background_colour, required_buttons_ }
	{
		setContentNonOwned(&content_component, true);
	}
	~AdditionalWindow() = default;

	void closeButtonPressed() override
	{
		setVisible(false);
	}

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdditionalWindow)
};
