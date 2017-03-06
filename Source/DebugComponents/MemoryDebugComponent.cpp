#include "../JuceLibraryCode/JuceHeader.h"
#include "MemoryDebugComponent.h"
#include "JucyBoy/Debug/DebugMMU.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

MemoryMapComponent::MemoryMapComponent(DebugMMU &debug_mmu) :
	debug_mmu_{ &debug_mmu }
{
	// Add memory map list header
	memory_map_list_header_.setJustificationType(Justification::centred);
	memory_map_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	memory_map_list_header_.setText("Memory Map", NotificationType::dontSendNotification);
	addAndMakeVisible(memory_map_list_header_);

	// Add memory map list
	memory_map_list_box_.setModel(this);
	addAndMakeVisible(memory_map_list_box_);
}

void MemoryMapComponent::UpdateMemoryMap(bool compute_diff)
{
	memory_map_ = debug_mmu_->GetMemoryMap();

	if (compute_diff)
	{
		std::transform(memory_map_.begin(), memory_map_.end(), previous_memory_map_state_.begin(), memory_map_colours_.begin(), [](size_t lhs, size_t rhs) -> Colour {return (lhs == rhs) ? Colours::black : Colours::red; });
	}
	else
	{
		std::fill(memory_map_colours_.begin(), memory_map_colours_.end(), Colours::black);
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
		row_text.append(value.str(), Font{ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain }, memory_map_colours_[16 * rowNumber + i]);
	}

	row_text.draw(g, Rectangle<int>{ width, height }.toFloat());
}

MemoryWatchpointsComponent::MemoryWatchpointsComponent(DebugMMU &debug_mmu) :
	debug_mmu_{ &debug_mmu }
{
	// Add list of watchpoints
	watchpoint_list_box_.setModel(this);
	addAndMakeVisible(watchpoint_list_box_);

	// Add watchpoints header
	watchpoint_list_header_.setJustificationType(Justification::centred);
	watchpoint_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	watchpoint_list_header_.setText("Watchpoints", NotificationType::dontSendNotification);
	addAndMakeVisible(watchpoint_list_header_);

	// Add text editor for new watchpoints
	watchpoint_add_editor_.addListener(this);
	watchpoint_add_editor_.setPopupMenuEnabled(false);
	watchpoint_add_editor_.setTextToShowWhenEmpty("Add watchpoint...", Colours::grey);
	watchpoint_add_editor_.setInputRestrictions(4, "0123456789ABCDEFabcdef");
	watchpoint_add_editor_.setIndents(static_cast<int>(watchpoint_add_editor_.getFont().getHeight()), 0);
	watchpoint_add_editor_.setColour(TextEditor::ColourIds::outlineColourId, Colours::orange);
	addAndMakeVisible(watchpoint_add_editor_);

	// Add radio buttons for watchpoint type
	watchpoint_type_read_.setRadioGroupId(1);
	watchpoint_type_write_.setRadioGroupId(1);
	watchpoint_type_write_.setToggleState(true, NotificationType::dontSendNotification);
	addAndMakeVisible(watchpoint_type_read_);
	addAndMakeVisible(watchpoint_type_write_);
}

void MemoryWatchpointsComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
	g.drawRect(watchpoint_add_area_, 1);
}

void MemoryWatchpointsComponent::resized()
{
	auto working_area = getLocalBounds();

	const auto watchpoint_list_area_height = working_area.getHeight();
	watchpoint_list_header_.setBounds(working_area.removeFromTop(watchpoint_list_area_height / 6));
	watchpoint_add_area_ = working_area.removeFromBottom(watchpoint_list_area_height / 6);
	watchpoint_list_box_.setBounds(working_area.reduced(1, 1));

	working_area = watchpoint_add_area_;
	watchpoint_add_editor_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	watchpoint_type_read_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	watchpoint_type_write_.setBounds(working_area);

	const auto vertical_indent = (watchpoint_add_editor_.getHeight() - watchpoint_add_editor_.getFont().getHeight()) / 2.0;
	watchpoint_add_editor_.setBorder(BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}

int MemoryWatchpointsComponent::getNumRows()
{
	return static_cast<int>(watchpoints_.size());
}

void MemoryWatchpointsComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= getNumRows())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	std::stringstream row_text;
	row_text << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << watchpoints_[rowNumber].address << " | ";
	switch (watchpoints_[rowNumber].type)
	{
	case Memory::Watchpoint::Type::Read:
		row_text << "Read";
		break;
	case Memory::Watchpoint::Type::Write:
		row_text << "Write";
		break;
	default:
		break;
	}

	g.drawText(row_text.str(), 0, 0, width, height, Justification::centred);
}

void MemoryWatchpointsComponent::textEditorReturnKeyPressed(TextEditor &)
{
	const auto watchpoint_address = std::stoi(watchpoint_add_editor_.getText().toStdString(), 0, 16);
	if (watchpoint_address < std::numeric_limits<Memory::Address>::min() || watchpoint_address > std::numeric_limits<Memory::Address>::max()) return;

	watchpoint_add_editor_.clear();

	if (watchpoint_type_read_.getToggleState())
	{
		debug_mmu_->AddWatchpoint(Memory::Watchpoint{ static_cast<Memory::Address>(watchpoint_address), Memory::Watchpoint::Type::Read });
	}
	else if (watchpoint_type_write_.getToggleState())
	{
		debug_mmu_->AddWatchpoint(Memory::Watchpoint{ static_cast<Memory::Address>(watchpoint_address), Memory::Watchpoint::Type::Write });
	}

	watchpoints_ = debug_mmu_->GetWatchpointList();

	watchpoint_list_box_.updateContent();
	watchpoint_list_box_.repaint();
}

void MemoryWatchpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	debug_mmu_->RemoveWatchpoint(watchpoints_[lastRowSelected]);

	watchpoints_ = debug_mmu_->GetWatchpointList();
	watchpoint_list_box_.updateContent();
	watchpoint_list_box_.repaint();
}

MemoryDebugComponent::MemoryDebugComponent(DebugMMU &debug_mmu) :
	memory_map_component_{ debug_mmu },
	memory_watchpoints_component_{ debug_mmu }
{
	addAndMakeVisible(memory_map_component_);
	addAndMakeVisible(memory_watchpoints_component_);
}

void MemoryDebugComponent::OnStatusUpdateRequested(bool compute_diff)
{
	memory_map_component_.UpdateMemoryMap(compute_diff);
}

void MemoryDebugComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void MemoryDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	memory_map_component_.setBounds(working_area.removeFromTop(3 * working_area.getHeight() / 4));
	memory_watchpoints_component_.setBounds(working_area);
}
