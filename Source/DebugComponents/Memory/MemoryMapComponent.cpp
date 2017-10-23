#include "../JuceLibraryCode/JuceHeader.h"
#include "MemoryMapComponent.h"
#include "../../JucyBoy/MMU.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

MemoryMapComponent::MemoryMapComponent()
{
	// Add memory map list header
	memory_map_list_header_.setJustificationType(juce::Justification::centred);
	memory_map_list_header_.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::orange);
	memory_map_list_header_.setText("Memory Map", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(memory_map_list_header_);

	// Add memory map list
	memory_map_list_box_.setModel(this);
	addAndMakeVisible(memory_map_list_box_);

	UpdateMemoryMap(false);
}

void MemoryMapComponent::OnEmulationStarted()
{
	is_emulation_running_ = true;
	memory_map_list_box_.repaint();
}

void MemoryMapComponent::OnEmulationPaused()
{
	is_emulation_running_ = false;
	memory_map_list_box_.repaint();
}

void MemoryMapComponent::UpdateMemoryMap(bool compute_diff)
{
	if (!mmu_) return;

	memory_map_ = mmu_->GetMemoryMap();

	if (compute_diff)
	{
		std::transform(memory_map_.begin(), memory_map_.end(), previous_memory_map_state_.begin(), memory_map_colours_.begin(),
			[](size_t lhs, size_t rhs) -> juce::Colour {return (lhs == rhs) ? juce::Colours::black : juce::Colours::red; });
	}
	else
	{
		std::fill(memory_map_colours_.begin(), memory_map_colours_.end(), juce::Colours::black);
	}

	memory_map_list_box_.repaint();

	previous_memory_map_state_ = memory_map_;
}

void MemoryMapComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void MemoryMapComponent::resized()
{
	auto working_area = getLocalBounds();
	memory_map_list_header_.setBounds(working_area.removeFromTop(30));
	memory_map_list_box_.setBounds(working_area.reduced(1, 1));
}

int MemoryMapComponent::getNumRows()
{
	return static_cast<int>(memory_map_.size() / 16);
}

void MemoryMapComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= getNumRows())) { return; }

	if (rowIsSelected)	g.fillAll(juce::Colours::lightblue);
	else				g.fillAll(juce::Colours::white);

	std::stringstream address;
	address << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << 16 * rowNumber << " ";
	juce::AttributedString row_text;
	row_text.append(address.str(), juce::Colours::grey);

	for (int i = 0; i < 16; ++i)
	{
		std::stringstream value;
		value << " " << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << static_cast<int>(memory_map_[16 * rowNumber + i]);
		row_text.append(value.str(), is_emulation_running_ ? juce::Colours::grey : memory_map_colours_[16 * rowNumber + i]);
	}

	row_text.setJustification(juce::Justification::centred);
	row_text.setFont(juce::Font{ juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain });
	row_text.draw(g, juce::Rectangle<int>{ width, height }.toFloat());
}
