#include "../JuceLibraryCode/JuceHeader.h"
#include "CpuStatusComponent.h"
#include <sstream>
#include <iomanip>
#include "JucyBoy/CPU.h"

//==============================================================================
CpuStatusComponent::CpuStatusComponent()
{
	// Add register labels
	addAndMakeVisible(af_label_);
	addAndMakeVisible(bc_label_);
	addAndMakeVisible(de_label_);
	addAndMakeVisible(hl_label_);
	addAndMakeVisible(pc_label_);
	addAndMakeVisible(sp_label_);

	af_label_.setJustificationType(Justification::centred);
	bc_label_.setJustificationType(Justification::centred);
	de_label_.setJustificationType(Justification::centred);
	hl_label_.setJustificationType(Justification::centred);
	pc_label_.setJustificationType(Justification::centred);
	sp_label_.setJustificationType(Justification::centred);

	// Add flag toggle buttons (read-only)
	addAndMakeVisible(carry_flag_toggle_);
	addAndMakeVisible(half_carry_flag_toggle_);
	addAndMakeVisible(subtract_flag_toggle_);
	addAndMakeVisible(zero_flag_toggle_);

	carry_flag_toggle_.setButtonText("C");
	carry_flag_toggle_.setEnabled(false);
	half_carry_flag_toggle_.setButtonText("H");
	half_carry_flag_toggle_.setEnabled(false);
	subtract_flag_toggle_.setButtonText("N");
	subtract_flag_toggle_.setEnabled(false);
	zero_flag_toggle_.setButtonText("Z");
	zero_flag_toggle_.setEnabled(false);
	
	// Add list of breakpoints
	breakpoint_list_box_.setModel(this);
	breakpoint_list_box_.setOutlineThickness(1);
	addAndMakeVisible(breakpoint_list_box_);
}

CpuStatusComponent::~CpuStatusComponent()
{

}

std::string CpuStatusComponent::FormatRegisterLabelText(std::string register_name, uint16_t value)
{
	std::stringstream register_text;
	register_text << std::move(register_name) << ": 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << value;
	return register_text.str();
}

void CpuStatusComponent::OnCpuStateChanged(const CPU::Registers &registers, CPU::Flags flags)
{
	af_label_.setText(FormatRegisterLabelText("AF", registers.af.Read()), NotificationType::dontSendNotification);
	bc_label_.setText(FormatRegisterLabelText("BC", registers.bc.Read()), NotificationType::dontSendNotification);
	de_label_.setText(FormatRegisterLabelText("DE", registers.de.Read()), NotificationType::dontSendNotification);
	hl_label_.setText(FormatRegisterLabelText("HL", registers.hl.Read()), NotificationType::dontSendNotification);
	pc_label_.setText(FormatRegisterLabelText("PC", registers.pc), NotificationType::dontSendNotification);
	sp_label_.setText(FormatRegisterLabelText("SP", registers.sp), NotificationType::dontSendNotification);

	carry_flag_toggle_.setToggleState((flags & CPU::Flags::C) != CPU::Flags::None, NotificationType::dontSendNotification);
	half_carry_flag_toggle_.setToggleState((flags & CPU::Flags::H) != CPU::Flags::None, NotificationType::dontSendNotification);
	subtract_flag_toggle_.setToggleState((flags & CPU::Flags::N) != CPU::Flags::None, NotificationType::dontSendNotification);
	zero_flag_toggle_.setToggleState((flags & CPU::Flags::Z) != CPU::Flags::None, NotificationType::dontSendNotification);
}

void CpuStatusComponent::OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list)
{
	breakpoints_ = std::vector<uint16_t>{ breakpoint_list.cbegin(), breakpoint_list.cend() };
	breakpoint_list_box_.updateContent();
}

void CpuStatusComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);   // clear the background

	g.setColour(Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
}

void CpuStatusComponent::resized()
{
	auto working_area = getLocalBounds();
	auto breakpoint_list_area = working_area.removeFromRight(getWidth() / 3);
	auto flags_area = working_area.removeFromBottom(getHeight() / 4);
	auto first_registers_row_area = working_area.removeFromTop(getHeight() / 4);
	auto second_registers_row_area = working_area.removeFromTop(getHeight() / 4);
	auto third_registers_row_area = working_area;

	af_label_.setBounds(first_registers_row_area.removeFromLeft(first_registers_row_area.getWidth() / 2));
	bc_label_.setBounds(first_registers_row_area);
	de_label_.setBounds(second_registers_row_area.removeFromLeft(second_registers_row_area.getWidth() / 2));
	hl_label_.setBounds(second_registers_row_area);
	pc_label_.setBounds(third_registers_row_area.removeFromLeft(third_registers_row_area.getWidth() / 2));
	sp_label_.setBounds(third_registers_row_area);

	const auto flags_area_width = flags_area.getWidth();
	zero_flag_toggle_.setBounds(flags_area.removeFromLeft(flags_area_width / 4));
	subtract_flag_toggle_.setBounds(flags_area.removeFromLeft(flags_area_width / 4));
	half_carry_flag_toggle_.setBounds(flags_area.removeFromLeft(flags_area_width / 4));
	carry_flag_toggle_.setBounds(flags_area);

	breakpoint_list_box_.setBounds(breakpoint_list_area);
}

int CpuStatusComponent::getNumRows()
{
	return static_cast<int>(breakpoints_.size());
}

void CpuStatusComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= breakpoints_.size())) { return; }

	g.drawText(FormatRegisterLabelText("PC", breakpoints_[rowNumber]), 0, 0, width, height, Justification::centred);
}
