#include "CpuBreakpointsComponent.h"
#include <sstream>
#include <iomanip>

CpuBreakpointsComponent::CpuBreakpointsComponent()
{
	// Add list of breakpoints
	breakpoint_list_box_.setModel(this);
	addAndMakeVisible(breakpoint_list_box_);

	// Add breakpoint list header
	breakpoint_list_header_.setJustificationType(juce::Justification::centred);
	breakpoint_list_header_.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::orange);
	breakpoint_list_header_.setText("Breakpoints", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(breakpoint_list_header_);

	// Add text editor for new breakpoints
	breakpoint_add_editor_.addListener(this);
	breakpoint_add_editor_.setPopupMenuEnabled(false);
	breakpoint_add_editor_.setTextToShowWhenEmpty("Add breakpoint...", juce::Colours::grey);
	breakpoint_add_editor_.setInputRestrictions(4, "0123456789ABCDEFabcdef");
	breakpoint_add_editor_.setIndents(static_cast<int>(breakpoint_add_editor_.getFont().getHeight()), 0);
	breakpoint_add_editor_.setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::orange);
	addAndMakeVisible(breakpoint_add_editor_);
}

void CpuBreakpointsComponent::SetCpu(DebugCPU& debug_cpu)
{
	debug_cpu_ = &debug_cpu;
	debug_cpu_->AddListener(*this);

	for (const auto &breakpoint : breakpoints_)
	{
		debug_cpu_->AddBreakpoint(breakpoint);
	}
}

void CpuBreakpointsComponent::OnEmulationStarted()
{
	// Settings enabled to false in text editors hides the colored outline, so do the following instead
	breakpoint_add_editor_.setReadOnly(true);
	breakpoint_add_editor_.setMouseClickGrabsKeyboardFocus(false);
	breakpoint_list_box_.deselectAllRows();
}

void CpuBreakpointsComponent::OnEmulationPaused()
{
	breakpoint_add_editor_.setReadOnly(false);
	breakpoint_add_editor_.setMouseClickGrabsKeyboardFocus(true);
}

void CpuBreakpointsComponent::OnBreakpointHit(Memory::Address breakpoint)
{
	juce::MessageManager::callAsync([this, breakpoint]() {
		const auto it = breakpoints_.find(breakpoint);
		if (it != breakpoints_.end())
		{
			breakpoint_list_box_.selectRow(static_cast<int>(std::distance(breakpoints_.begin(), it)));
		}
	});
}

int CpuBreakpointsComponent::getNumRows()
{
	return static_cast<int>(breakpoints_.size());
}

void CpuBreakpointsComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= breakpoints_.size())) { return; }

	if (rowIsSelected)	g.fillAll(juce::Colours::lightblue);
	else				g.fillAll(juce::Colours::white);

	auto it = breakpoints_.begin();
	std::advance(it, rowNumber);

	std::stringstream breakpoint_string;
	breakpoint_string << "PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << *it;

	g.drawText(breakpoint_string.str(), 0, 0, width, height, juce::Justification::centred);
}

void CpuBreakpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	if (lastRowSelected >= breakpoints_.size()) return;

	auto it = breakpoints_.begin();
	std::advance(it, lastRowSelected);

	if (debug_cpu_) debug_cpu_->RemoveBreakpoint(*it);

	breakpoints_.erase(it);

	breakpoint_list_box_.updateContent();
	breakpoint_list_box_.deselectAllRows();
}

void CpuBreakpointsComponent::textEditorReturnKeyPressed(juce::TextEditor&)
{
	const auto breakpoint = std::stoi(breakpoint_add_editor_.getText().toStdString(), 0, 16);
	if (breakpoint < std::numeric_limits<uint16_t>::min() || breakpoint > std::numeric_limits<uint16_t>::max()) return;

	const auto insert_result = breakpoints_.insert(static_cast<uint16_t>(breakpoint));
	if (insert_result.second && debug_cpu_) debug_cpu_->AddBreakpoint(*insert_result.first);

	breakpoint_add_editor_.clear();
	breakpoint_list_box_.updateContent();
}

void CpuBreakpointsComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuBreakpointsComponent::resized()
{
	auto working_area = getLocalBounds();

	breakpoint_list_header_.setBounds(working_area.removeFromTop(static_cast<int>(breakpoint_list_header_.getFont().getHeight() * 1.5)));
	breakpoint_add_editor_.setBounds(working_area.removeFromBottom(static_cast<int>(breakpoint_add_editor_.getFont().getHeight() * 1.5)));
	breakpoint_list_box_.setBounds(working_area.reduced(1, 0));

	const auto vertical_indent = (breakpoint_add_editor_.getHeight() - breakpoint_add_editor_.getFont().getHeight()) / 2.0;
	breakpoint_add_editor_.setBorder(juce::BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}
