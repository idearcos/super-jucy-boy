#include "WatchpointsComponent.h"
#include "../JucyBoy/Debug/DebugCPU.h"
#include <sstream>
#include <iomanip>

WatchpointsComponent::WatchpointsComponent()
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

void WatchpointsComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
	g.drawRect(watchpoint_add_area_, 1);
}

void WatchpointsComponent::resized()
{
	auto working_area = getLocalBounds();

	const auto watchpoint_list_area_height = working_area.getHeight();
	watchpoint_list_header_.setBounds(working_area.removeFromTop(watchpoint_list_header_.getFont().getHeight() * 1.5));
	watchpoint_add_area_ = working_area.removeFromBottom(watchpoint_list_header_.getFont().getHeight() * 1.5);
	watchpoint_list_box_.setBounds(working_area.reduced(1, 1));

	working_area = watchpoint_add_area_;
	watchpoint_add_editor_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	watchpoint_type_read_.setBounds(working_area.removeFromLeft(working_area.getWidth() / 2));
	watchpoint_type_write_.setBounds(working_area);

	const auto vertical_indent = (watchpoint_add_editor_.getHeight() - watchpoint_add_editor_.getFont().getHeight()) / 2.0;
	watchpoint_add_editor_.setBorder(BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}

int WatchpointsComponent::getNumRows()
{
	return static_cast<int>(watchpoints_.size());
}

void WatchpointsComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
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

void WatchpointsComponent::textEditorReturnKeyPressed(TextEditor &)
{
	const auto watchpoint_address = std::stoi(watchpoint_add_editor_.getText().toStdString(), 0, 16);
	if (watchpoint_address < std::numeric_limits<Memory::Address>::min() || watchpoint_address > std::numeric_limits<Memory::Address>::max()) return;

	watchpoint_add_editor_.clear();

	if (watchpoint_type_read_.getToggleState())
	{
		debug_cpu_->AddWatchpoint(Memory::Watchpoint{ static_cast<Memory::Address>(watchpoint_address), Memory::Watchpoint::Type::Read });
	}
	else if (watchpoint_type_write_.getToggleState())
	{
		debug_cpu_->AddWatchpoint(Memory::Watchpoint{ static_cast<Memory::Address>(watchpoint_address), Memory::Watchpoint::Type::Write });
	}

	watchpoints_ = debug_cpu_->GetWatchpointList();

	watchpoint_list_box_.updateContent();
	watchpoint_list_box_.repaint();
}

void WatchpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	debug_cpu_->RemoveWatchpoint(watchpoints_[lastRowSelected]);

	watchpoints_ = debug_cpu_->GetWatchpointList();
	watchpoint_list_box_.updateContent();
	watchpoint_list_box_.repaint();
}
