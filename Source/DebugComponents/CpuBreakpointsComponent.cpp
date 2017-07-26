#include "CpuBreakpointsComponent.h"
#include <sstream>
#include <iomanip>

CpuBreakpointsComponent::CpuBreakpointsComponent(DebugCPU& debug_cpu) :
	debug_cpu_{ &debug_cpu }
{
	debug_cpu_->AddListener(*this);

	// Add list of breakpoints
	breakpoint_list_box_.setModel(this);
	addAndMakeVisible(breakpoint_list_box_);

	// Add breakpoint list header
	breakpoint_list_header_.setJustificationType(Justification::centred);
	breakpoint_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	breakpoint_list_header_.setText("Breakpoints", NotificationType::dontSendNotification);
	addAndMakeVisible(breakpoint_list_header_);

	// Add text editor for new breakpoints
	breakpoint_add_editor_.addListener(this);
	breakpoint_add_editor_.setPopupMenuEnabled(false);
	breakpoint_add_editor_.setTextToShowWhenEmpty("Add breakpoint...", Colours::grey);
	breakpoint_add_editor_.setInputRestrictions(4, "0123456789ABCDEFabcdef");
	breakpoint_add_editor_.setIndents(static_cast<int>(breakpoint_add_editor_.getFont().getHeight()), 0);
	breakpoint_add_editor_.setColour(TextEditor::ColourIds::outlineColourId, Colours::orange);
	addAndMakeVisible(breakpoint_add_editor_);
}

void CpuBreakpointsComponent::OnEmulationStarted()
{
	// Settings enabled to false in text editors hides the colored outline, so do the following instead
	breakpoint_add_editor_.setReadOnly(true);
	breakpoint_add_editor_.setMouseClickGrabsKeyboardFocus(false);
}

void CpuBreakpointsComponent::OnEmulationPaused()
{
	breakpoint_add_editor_.setReadOnly(false);
	breakpoint_add_editor_.setMouseClickGrabsKeyboardFocus(true);
}

void CpuBreakpointsComponent::OnBreakpointHit(Memory::Address breakpoint)
{
	MessageManager::callAsync([this, breakpoint]() {
		for (int i = 0; i < breakpoints_.size(); ++i)
		{
			if (breakpoints_[i] == breakpoint) breakpoint_list_box_.selectRow(i);
		}
	});
}

int CpuBreakpointsComponent::getNumRows()
{
	return static_cast<int>(breakpoints_.size());
}

void CpuBreakpointsComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= breakpoints_.size())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	std::stringstream breakpoint_string;
	breakpoint_string << "PC: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << breakpoints_[rowNumber];

	g.drawText(breakpoint_string.str(), 0, 0, width, height, Justification::centred);
}

void CpuBreakpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	debug_cpu_->RemoveBreakpoint(breakpoints_[lastRowSelected]);
	UpdateBreakpoints();
}

void CpuBreakpointsComponent::textEditorReturnKeyPressed(TextEditor&)
{
	const auto breakpoint = std::stoi(breakpoint_add_editor_.getText().toStdString(), 0, 16);
	if (breakpoint < std::numeric_limits<Memory::Address>::min() || breakpoint > std::numeric_limits<Memory::Address>::max()) return;

	breakpoint_add_editor_.clear();

	debug_cpu_->AddBreakpoint(static_cast<Memory::Address>(breakpoint));
	UpdateBreakpoints();
}

void CpuBreakpointsComponent::UpdateBreakpoints()
{
	const auto cpu_breakpoints = debug_cpu_->GetBreakpoints();
	breakpoints_ = std::vector<Memory::Address>{ cpu_breakpoints.cbegin(), cpu_breakpoints.cend() };
	breakpoint_list_box_.updateContent();
}

void CpuBreakpointsComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuBreakpointsComponent::resized()
{
	auto working_area = getLocalBounds();

	breakpoint_list_header_.setBounds(working_area.removeFromTop(breakpoint_list_header_.getFont().getHeight() * 1.5));
	breakpoint_add_editor_.setBounds(working_area.removeFromBottom(breakpoint_add_editor_.getFont().getHeight() * 1.5));
	breakpoint_list_box_.setBounds(working_area.reduced(1, 0));

	const auto vertical_indent = (breakpoint_add_editor_.getHeight() - breakpoint_add_editor_.getFont().getHeight()) / 2.0;
	breakpoint_add_editor_.setBorder(BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}
