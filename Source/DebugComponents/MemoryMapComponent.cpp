#include "../JuceLibraryCode/JuceHeader.h"
#include "MemoryMapComponent.h"
#include "../JucyBoy/MMU.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

MemoryMapComponent::MemoryMapComponent(MMU &mmu) :
	mmu_{ &mmu }
{
	// Add memory map list header
	memory_map_list_header_.setJustificationType(Justification::centred);
	memory_map_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	memory_map_list_header_.setText("Memory Map", NotificationType::dontSendNotification);
	addAndMakeVisible(memory_map_list_header_);

	// Add memory map list
	memory_map_list_box_.setModel(this);
	addAndMakeVisible(memory_map_list_box_);

	UpdateState(false);
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

void MemoryMapComponent::UpdateState(bool compute_diff)
{
	memory_map_ = mmu_->GetMemoryMap();

	if (compute_diff)
	{
		std::transform(memory_map_.begin(), memory_map_.end(), previous_memory_map_state_.begin(), memory_map_colours_.begin(), [](size_t lhs, size_t rhs) -> Colour {return (lhs == rhs) ? Colours::black : Colours::red; });
	}
	else
	{
		std::fill(memory_map_colours_.begin(), memory_map_colours_.end(), Colours::black);
	}

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
	const auto total_height = working_area.getHeight();
	memory_map_list_header_.setBounds(working_area.removeFromTop(30));
	memory_map_list_box_.setBounds(working_area.reduced(1, 1));
}

int MemoryMapComponent::getNumRows()
{
	return static_cast<int>(memory_map_.size() / 16);
}

void MemoryMapComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= getNumRows())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	AttributedString row_text;
	row_text.setJustification(Justification::centred);

	std::stringstream address;
	address << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << 16 * rowNumber << " ";
	row_text.append(address.str(), Font{ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain }, Colours::grey);

	for (int i = 0; i < 16; ++i)
	{
		std::stringstream value;
		value << " " << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << static_cast<int>(memory_map_[16 * rowNumber + i]);
		row_text.append(value.str(), Font{ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain }, is_emulation_running_ ? Colours::grey : memory_map_colours_[16 * rowNumber + i]);
	}

	row_text.draw(g, Rectangle<int>{ width, height }.toFloat());
}
