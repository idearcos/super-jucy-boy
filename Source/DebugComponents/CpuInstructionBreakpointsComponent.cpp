#include "CpuInstructionBreakpointsComponent.h"
#include <sstream>
#include <iomanip>
#include "../JucyBoy/InstructionMnemonics.h"

CpuInstructionBreakpointsComponent::CpuInstructionBreakpointsComponent()
{
	// Add list of instruction breakpoints
	instruction_breakpoint_list_box_.setModel(this);
	addAndMakeVisible(instruction_breakpoint_list_box_);

	// Add instruction breakpoint list header
	instruction_breakpoint_list_header_.setJustificationType(juce::Justification::centred);
	instruction_breakpoint_list_header_.setColour(juce::Label::ColourIds::outlineColourId, juce::Colours::orange);
	instruction_breakpoint_list_header_.setText("Instruction breakpoints", juce::NotificationType::dontSendNotification);
	addAndMakeVisible(instruction_breakpoint_list_header_);

	// Add combo box for new instruction breakpoints
	for (int ii = 0; ii <= 255; ++ii)
	{
		if ((ii % 16) == 0)
		{
			instruction_breakpoint_add_combo_box_.addSeparator();
		}

		std::stringstream instruction_string;
		instruction_string << "[0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << ii << "] ";
		instruction_string << GetInstructionMnemonic(ii);
		instruction_breakpoint_add_combo_box_.addItem(instruction_string.str(), ii + 1);
	}
	instruction_breakpoint_add_combo_box_.addListener(this);
	addAndMakeVisible(instruction_breakpoint_add_combo_box_);
	instruction_breakpoint_add_combo_box_.setTextWhenNothingSelected("Select opcode");

	// Add button for new instruction breakpoints
	instruction_breakpoint_add_button_.setButtonText("Add");
	instruction_breakpoint_add_button_.addListener(this);
	addAndMakeVisible(instruction_breakpoint_add_button_);
}

void CpuInstructionBreakpointsComponent::SetCpu(DebugCPU& debug_cpu)
{
	debug_cpu_ = &debug_cpu;
	debug_cpu_->AddListener(*this);

	for (const auto &instruction_breakpoint : instruction_breakpoints_)
	{
		debug_cpu_->AddInstructionBreakpoint(instruction_breakpoint);
	}
}

void CpuInstructionBreakpointsComponent::OnEmulationStarted()
{
	instruction_breakpoint_add_combo_box_.setEnabled(false);
	instruction_breakpoint_add_button_.setEnabled(false);
	instruction_breakpoint_list_box_.deselectAllRows();
}

void CpuInstructionBreakpointsComponent::OnEmulationPaused()
{
	instruction_breakpoint_add_combo_box_.setEnabled(true);
	instruction_breakpoint_add_button_.setEnabled(true);
}

void CpuInstructionBreakpointsComponent::OnInstructionBreakpointHit(CPU::OpCode opcode)
{
	juce::MessageManager::callAsync([this, opcode]() {
		const auto it = instruction_breakpoints_.find(opcode);
		if (it != instruction_breakpoints_.end())
		{
			instruction_breakpoint_list_box_.selectRow(static_cast<int>(std::distance(instruction_breakpoints_.begin(), it)));
		}
	});
}

int CpuInstructionBreakpointsComponent::getNumRows()
{
	return static_cast<int>(instruction_breakpoints_.size());
}

void CpuInstructionBreakpointsComponent::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= instruction_breakpoints_.size())) { return; }

	if (rowIsSelected)	g.fillAll(juce::Colours::lightblue);
	else				g.fillAll(juce::Colours::white);

	auto it = instruction_breakpoints_.begin();
	std::advance(it, rowNumber);

	g.drawText(instruction_breakpoint_add_combo_box_.getItemText(*it), 0, 0, width, height, juce::Justification::centred);
}

void CpuInstructionBreakpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	if (lastRowSelected >= instruction_breakpoints_.size()) return;

	auto it = instruction_breakpoints_.begin();
	std::advance(it, lastRowSelected);

	if (debug_cpu_) debug_cpu_->RemoveInstructionBreakpoint(*it);

	instruction_breakpoints_.erase(it);

	instruction_breakpoint_list_box_.updateContent();
	instruction_breakpoint_list_box_.deselectAllRows();
}

void CpuInstructionBreakpointsComponent::buttonClicked(juce::Button*)
{
	const auto insert_result = instruction_breakpoints_.insert(static_cast<CPU::OpCode>(instruction_breakpoint_add_combo_box_.getSelectedId() - 1));
	if (insert_result.second && debug_cpu_) debug_cpu_->AddInstructionBreakpoint(*insert_result.first);

	instruction_breakpoint_list_box_.updateContent();
}

void CpuInstructionBreakpointsComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuInstructionBreakpointsComponent::resized()
{
	auto working_area = getLocalBounds();

	instruction_breakpoint_list_header_.setBounds(working_area.removeFromTop(instruction_breakpoint_list_header_.getFont().getHeight() * 1.5));
	auto addition_controls_area = working_area.removeFromBottom(instruction_breakpoint_list_header_.getFont().getHeight() * 1.5);
	instruction_breakpoint_list_box_.setBounds(working_area.reduced(1, 0));

	instruction_breakpoint_add_combo_box_.setBounds(addition_controls_area.removeFromLeft(3 * addition_controls_area.getWidth() / 4));
	instruction_breakpoint_add_button_.setBounds(addition_controls_area);
}
