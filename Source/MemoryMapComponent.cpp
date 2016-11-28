#include "../JuceLibraryCode/JuceHeader.h"
#include "MemoryMapComponent.h"
#include <sstream>
#include <iomanip>

MemoryMapComponent::MemoryMapComponent(MMU &mmu) :
	mmu_(&mmu)
{
	// Add memory map list
	memory_map_list_box_.setModel(this);
	addAndMakeVisible(memory_map_list_box_);

	// Add memory map list header
	memory_map_list_header_.setJustificationType(Justification::centred);
	memory_map_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	memory_map_list_header_.setText("Memory Map", NotificationType::dontSendNotification);
	addAndMakeVisible(memory_map_list_header_);
}

MemoryMapComponent::~MemoryMapComponent()
{

}

std::string MemoryMapComponent::FormatListBoxRowText(size_t memory_address, uint8_t value)
{
	std::stringstream register_text;
	register_text << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << memory_address
		<< ": " << std::setw(2) << static_cast<int>(value);
	return register_text.str();
}

void MemoryMapComponent::OnStatusUpdateRequested(bool compute_diff)
{
	memory_map_ = mmu_->GetMemoryMap();

	if (compute_diff)
	{
		for (int i = 0; i < memory_map_.size(); ++i)
		{
			memory_map_colours_[i] = memory_map_[i] != previous_memory_map_state_[i] ? Colours::red : Colours::black;
		}
	}
	else
	{
		for (int i = 0; i < memory_map_.size(); ++i)
		{
			memory_map_colours_[i] = Colours::black;
		}
	}

	memory_map_list_box_.updateContent();
	memory_map_list_box_.repaint();

	previous_memory_map_state_ = memory_map_;
}

void MemoryMapComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
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
	return static_cast<int>(memory_map_.size());
}

void MemoryMapComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= memory_map_.size())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	g.setColour(memory_map_colours_[rowNumber]);

	g.drawText(FormatListBoxRowText(rowNumber, memory_map_[rowNumber]), 0, 0, width, height, Justification::centred);
}
