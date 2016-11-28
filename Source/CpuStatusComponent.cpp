#include "CpuStatusComponent.h"
#include <sstream>
#include <iomanip>

CpuStatusComponent::CpuStatusComponent(CPU &cpu) :
	cpu_(&cpu)
{
	// Add register labels
	af_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(af_label_);
	bc_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(bc_label_);
	de_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(de_label_);
	hl_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(hl_label_);
	sp_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(sp_label_);
	pc_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(pc_label_);

	// Add flag toggle buttons (read-only)
	carry_flag_toggle_.setButtonText("C");
	carry_flag_toggle_.setEnabled(false);
	addAndMakeVisible(carry_flag_toggle_);

	half_carry_flag_toggle_.setButtonText("H");
	half_carry_flag_toggle_.setEnabled(false);
	addAndMakeVisible(half_carry_flag_toggle_);

	subtract_flag_toggle_.setButtonText("N");
	subtract_flag_toggle_.setEnabled(false);
	addAndMakeVisible(subtract_flag_toggle_);

	zero_flag_toggle_.setButtonText("Z");
	zero_flag_toggle_.setEnabled(false);
	addAndMakeVisible(zero_flag_toggle_);

	// Add list of breakpoints
	breakpoint_list_box_.setModel(this);
	addAndMakeVisible(breakpoint_list_box_);

	// Add breakpoints header
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

CpuStatusComponent::~CpuStatusComponent()
{

}

std::string CpuStatusComponent::FormatRegisterLabelText(std::string register_name, uint16_t value)
{
	std::stringstream register_text;
	register_text << std::move(register_name) << ": 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << value;
	return register_text.str();
}

void CpuStatusComponent::OnStatusUpdateRequested(bool compute_diff)
{
	const auto registers = cpu_->GetRegistersState();
	const auto cpu_flags = cpu_->GetFlagsState();

	af_label_.setText(FormatRegisterLabelText("AF", registers.af), NotificationType::dontSendNotification);
	bc_label_.setText(FormatRegisterLabelText("BC", registers.bc), NotificationType::dontSendNotification);
	de_label_.setText(FormatRegisterLabelText("DE", registers.de), NotificationType::dontSendNotification);
	hl_label_.setText(FormatRegisterLabelText("HL", registers.hl), NotificationType::dontSendNotification);
	sp_label_.setText(FormatRegisterLabelText("SP", registers.sp), NotificationType::dontSendNotification);
	pc_label_.setText(FormatRegisterLabelText("PC", registers.pc), NotificationType::dontSendNotification);

	carry_flag_toggle_.setToggleState((cpu_flags & CPU::Flags::C) != CPU::Flags::None, NotificationType::dontSendNotification);
	half_carry_flag_toggle_.setToggleState((cpu_flags & CPU::Flags::H) != CPU::Flags::None, NotificationType::dontSendNotification);
	subtract_flag_toggle_.setToggleState((cpu_flags & CPU::Flags::N) != CPU::Flags::None, NotificationType::dontSendNotification);
	zero_flag_toggle_.setToggleState((cpu_flags & CPU::Flags::Z) != CPU::Flags::None, NotificationType::dontSendNotification);

	if (compute_diff)
	{
		af_label_.setColour(Label::ColourIds::textColourId, (registers.af != previous_registers_state_.af) ? Colours::red : Colours::black);
		bc_label_.setColour(Label::ColourIds::textColourId, (registers.bc != previous_registers_state_.bc) ? Colours::red : Colours::black);
		de_label_.setColour(Label::ColourIds::textColourId, (registers.de != previous_registers_state_.de) ? Colours::red : Colours::black);
		hl_label_.setColour(Label::ColourIds::textColourId, (registers.hl != previous_registers_state_.hl) ? Colours::red : Colours::black);
		sp_label_.setColour(Label::ColourIds::textColourId, (registers.sp != previous_registers_state_.sp) ? Colours::red : Colours::black);
		pc_label_.setColour(Label::ColourIds::textColourId, (registers.pc != previous_registers_state_.pc) ? Colours::red : Colours::black);

		carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, ((cpu_flags & CPU::Flags::C) != (previous_cpu_flags_state_ & CPU::Flags::C)) ? Colours::red : Colours::black);
		half_carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, ((cpu_flags & CPU::Flags::H) != (previous_cpu_flags_state_ & CPU::Flags::H)) ? Colours::red : Colours::black);
		subtract_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, ((cpu_flags & CPU::Flags::N) != (previous_cpu_flags_state_ & CPU::Flags::N)) ? Colours::red : Colours::black);
		zero_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, ((cpu_flags & CPU::Flags::Z) != (previous_cpu_flags_state_ & CPU::Flags::Z)) ? Colours::red : Colours::black);
	}
	else
	{
		af_label_.setColour(Label::ColourIds::textColourId, Colours::black);
		bc_label_.setColour(Label::ColourIds::textColourId, Colours::black);
		de_label_.setColour(Label::ColourIds::textColourId, Colours::black);
		hl_label_.setColour(Label::ColourIds::textColourId, Colours::black);
		sp_label_.setColour(Label::ColourIds::textColourId, Colours::black);
		pc_label_.setColour(Label::ColourIds::textColourId, Colours::black);

		carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
		half_carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
		subtract_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
		zero_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
	}

	previous_registers_state_ = registers;
	previous_cpu_flags_state_ = cpu_flags;

	for (int i = 0; i < breakpoints_.size(); ++i)
	{
		if (breakpoints_[i] == registers.pc) breakpoint_list_box_.selectRow(i);
	}
}

void CpuStatusComponent::OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list)
{
	breakpoints_ = std::vector<uint16_t>{ breakpoint_list.cbegin(), breakpoint_list.cend() };
	breakpoint_list_box_.updateContent();
}

void CpuStatusComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuStatusComponent::resized()
{
	auto working_area = getLocalBounds();
	auto registers_area = working_area.removeFromTop(getHeight() / 3);
	auto flags_area = registers_area.removeFromRight(getWidth() / 3);

	const auto registers_area_height = registers_area.getHeight();
	af_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));
	bc_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));
	de_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));
	hl_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));
	sp_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));
	pc_label_.setBounds(registers_area.removeFromTop(registers_area_height / 6));

	const auto flags_area_height = flags_area.getHeight();
	zero_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	subtract_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	half_carry_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	carry_flag_toggle_.setBounds(flags_area);

	auto breakpoint_list_area_height = working_area.getHeight();
	breakpoint_list_header_.setBounds(working_area.removeFromTop(breakpoint_list_area_height / 10));
	breakpoint_add_editor_.setBounds(working_area.removeFromBottom(breakpoint_list_area_height / 10));
	breakpoint_list_box_.setBounds(working_area.reduced(1, 0));

	const auto vertical_indent = (breakpoint_add_editor_.getHeight() - breakpoint_add_editor_.getFont().getHeight()) / 2.0;
	breakpoint_add_editor_.setBorder(BorderSize<int>{ static_cast<int>(vertical_indent), 0, 0, 0 });
}

int CpuStatusComponent::getNumRows()
{
	return static_cast<int>(breakpoints_.size());
}

void CpuStatusComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
	if ((rowNumber < 0) || (rowNumber >= breakpoints_.size())) { return; }

	if (rowIsSelected)	g.fillAll(Colours::lightblue);
	else				g.fillAll(Colours::white);

	g.drawText(FormatRegisterLabelText("PC", breakpoints_[rowNumber]), 0, 0, width, height, Justification::centred);
}

void CpuStatusComponent::deleteKeyPressed(int lastRowSelected)
{
	cpu_->RemoveBreakpoint(breakpoints_[lastRowSelected]);
}

void CpuStatusComponent::textEditorReturnKeyPressed(TextEditor &)
{
	const auto breakpoint = std::stoi(breakpoint_add_editor_.getText().toStdString(), 0, 16);
	if (breakpoint < std::numeric_limits<Memory::Address>::min() || breakpoint > std::numeric_limits<Memory::Address>::max()) return;

	breakpoint_add_editor_.clear();

	cpu_->AddBreakpoint(static_cast<Memory::Address>(breakpoint));
}
