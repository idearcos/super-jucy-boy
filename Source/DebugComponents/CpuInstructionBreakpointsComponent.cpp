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
	instruction_breakpoint_list_header_.setJustificationType(Justification::centred);
	instruction_breakpoint_list_header_.setColour(Label::ColourIds::outlineColourId, Colours::orange);
	instruction_breakpoint_list_header_.setText("Instruction breakpoints", NotificationType::dontSendNotification);
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

CpuInstructionBreakpointsComponent::~CpuInstructionBreakpointsComponent()
{
	
}

void CpuInstructionBreakpointsComponent::SetCpu(DebugCPU& cpu)
{
	cpu_ = &cpu;
	cpu_->AddListener(*this);
}

int CpuInstructionBreakpointsComponent::getNumRows()
{
	return static_cast<int>(instruction_breakpoints_.size());
}

void CpuInstructionBreakpointsComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= instruction_breakpoints_.size())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	g.drawText(instruction_breakpoint_add_combo_box_.getItemText(instruction_breakpoints_[rowNumber]), 0, 0, width, height, Justification::centred);
}

void CpuInstructionBreakpointsComponent::deleteKeyPressed(int lastRowSelected)
{
	if (!cpu_) return;

	cpu_->RemoveInstructionBreakpoint(instruction_breakpoints_[lastRowSelected]);
	UpdateInstructionBreakpoints();
}

void CpuInstructionBreakpointsComponent::buttonClicked(Button*)
{
	if (!cpu_) return;

	cpu_->AddInstructionBreakpoint(static_cast<CPU::OpCode>(instruction_breakpoint_add_combo_box_.getSelectedId() - 1));
	UpdateInstructionBreakpoints();
}

void CpuInstructionBreakpointsComponent::UpdateInstructionBreakpoints()
{
	if (!cpu_) return;

	const auto cpu_instruction_breakpoints = cpu_->GetInstructionBreakpoints();
	instruction_breakpoints_ = std::vector<CPU::OpCode>{ cpu_instruction_breakpoints.cbegin(), cpu_instruction_breakpoints.cend() };
	instruction_breakpoint_list_box_.updateContent();
}

void CpuInstructionBreakpointsComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuInstructionBreakpointsComponent::resized()
{
	auto working_area = getLocalBounds();
	auto working_area_height = working_area.getHeight();

	instruction_breakpoint_list_header_.setBounds(working_area.removeFromTop(instruction_breakpoint_list_header_.getFont().getHeight() * 1.5));
	auto addition_controls_area = working_area.removeFromBottom(instruction_breakpoint_list_header_.getFont().getHeight() * 1.5);
	instruction_breakpoint_list_box_.setBounds(working_area.reduced(1, 0));

	instruction_breakpoint_add_combo_box_.setBounds(addition_controls_area.removeFromLeft(3 * addition_controls_area.getWidth() / 4));
	instruction_breakpoint_add_button_.setBounds(addition_controls_area);
}

void CpuInstructionBreakpointsComponent::UpdateHitInstructionBreakpoint(CPU::OpCode opcode)
{
	for (int i = 0; i < instruction_breakpoints_.size(); ++i)
	{
		if (instruction_breakpoints_[i] == opcode) instruction_breakpoint_list_box_.selectRow(i);
	}
}
