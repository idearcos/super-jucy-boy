#include "WatchpointsComponent.h"
#include <sstream>
#include <iomanip>

WatchpointsComponent::WatchpointsComponent()
{
	// Add list of watchpoints
	watchpoint_list_box_.setModel(this);
	addAndMakeVisible(watchpoint_list_box_);

	// Add watchpoints header
	watchpoint_list_header_.setJustificationType(juce::Justification::centred);
	watchpoint_list_header_.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::orange);
	watchpoint_list_header_.setText("Watchpoints", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(watchpoint_list_header_);

	// Add text editor for new watchpoints
	watchpoint_add_editor_.addListener(this);
	watchpoint_add_editor_.setPopupMenuEnabled(false);
	watchpoint_add_editor_.setTextToShowWhenEmpty("Add watchpoint...", juce::Colours::grey);
	watchpoint_add_editor_.setInputRestrictions(4, "0123456789ABCDEFabcdef");
	watchpoint_add_editor_.setIndents(static_cast<int>(watchpoint_add_editor_.getFont().getHeight()), 0);
	watchpoint_add_editor_.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::orange);
	addAndMakeVisible(watchpoint_add_editor_);

	// Add radio buttons for watchpoint type
	watchpoint_type_read_.setRadioGroupId(1);
	watchpoint_type_write_.setRadioGroupId(1);
	watchpoint_type_write_.setToggleState(true, juce::NotificationType::dontSendNotification);
	addAndMakeVisible(watchpoint_type_read_);
	addAndMakeVisible(watchpoint_type_write_);
}

void WatchpointsComponent::SetCpu(DebugCPU& debug_cpu)
{
	debug_cpu_ = &debug_cpu;
	debug_cpu_->AddListener(*this);

	for (const auto &watchpoint : watchpoints_)
	{
		debug_cpu_->AddWatchpoint(watchpoint);
	}
}

void WatchpointsComponent::OnEmulationStarted()
{
	// Settings enabled to false in text editors hides the colored outline, so do the following instead
	watchpoint_add_editor_.setReadOnly(true);
	watchpoint_add_editor_.setMouseClickGrabsKeyboardFocus(false);
	watchpoint_type_read_.setEnabled(false);
	watchpoint_type_write_.setEnabled(false);
	watchpoint_list_box_.deselectAllRows();
}

void WatchpointsComponent::OnEmulationPaused()
{
	watchpoint_add_editor_.setReadOnly(false);
	watchpoint_add_editor_.setMouseClickGrabsKeyboardFocus(true);
	watchpoint_type_read_.setEnabled(true);
	watchpoint_type_write_.setEnabled(true);
}

void WatchpointsComponent::OnWatchpointHit(Memory::Watchpoint watchpoint)
{
	juce::MessageManager::callAsync([this, watchpoint]() {
		const auto it = watchpoints_.find(watchpoint);
		if (it != watchpoints_.end())
		{
			watchpoint_list_box_.selectRow(static_cast<int>(std::distance(watchpoints_.begin(), it)));
		}
	});
}

void WatchpointsComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
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
	watchpoint_add_editor_.setBorder(juce::BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}

int WatchpointsComponent::getNumRows()
{
	return static_cast<int>(watchpoints_.size());
}

void WatchpointsComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= getNumRows())) { return; }

	if (rowIsSelected)	g.fillAll(juce::Colours::lightblue);
	else				g.fillAll(juce::Colours::white);

	auto it = watchpoints_.begin();
	std::advance(it, rowNumber);

	std::stringstream row_text;
	row_text << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << it->address.GetAbsolute() << " | ";
	switch (it->type)
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

	g.drawText(row_text.str(), 0, 0, width, height, juce::Justification::centred);
}

void WatchpointsComponent::textEditorReturnKeyPressed(juce::TextEditor &)
{
	const auto watchpoint_address = std::stoi(watchpoint_add_editor_.getText().toStdString(), 0, 16);
	if (watchpoint_address < std::numeric_limits<uint16_t>::min() || watchpoint_address > std::numeric_limits<uint16_t>::max()) return;

	std::unique_ptr<Memory::Watchpoint> watchpoint;
	if (watchpoint_type_read_.getToggleState())
	{
		watchpoint = std::make_unique<Memory::Watchpoint>(watchpoint_address, Memory::Watchpoint::Type::Read);
	}
	else if (watchpoint_type_write_.getToggleState())
	{
		watchpoint = std::make_unique<Memory::Watchpoint>(watchpoint_address, Memory::Watchpoint::Type::Write);
	}
	if (!watchpoint) return;

	const auto insert_result = watchpoints_.insert(*watchpoint);
	if (insert_result.second && debug_cpu_) debug_cpu_->AddWatchpoint(*insert_result.first);

	watchpoint_add_editor_.clear();
	watchpoint_list_box_.updateContent();
}

void WatchpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	if (lastRowSelected >= watchpoints_.size()) return;

	auto it = watchpoints_.begin();
	std::advance(it, lastRowSelected);

	if (debug_cpu_) debug_cpu_->RemoveWatchpoint(*it);

	watchpoints_.erase(it);

	watchpoint_list_box_.updateContent();
	watchpoint_list_box_.deselectAllRows();
}
