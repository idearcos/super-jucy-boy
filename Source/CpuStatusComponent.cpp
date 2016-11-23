#include "CpuStatusComponent.h"
#include <sstream>
#include <iomanip>

CpuStatusComponent::CpuStatusComponent()
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
	pc_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(pc_label_);
	sp_label_.setJustificationType(Justification::centred);
	addAndMakeVisible(sp_label_);

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
	//breakpoint_list_box_.setOutlineThickness(1);
	addAndMakeVisible(breakpoint_list_box_);

	// Add text editor for new breakpoints
	breakpoint_add_editor_.addListener(this);
	breakpoint_add_editor_.setPopupMenuEnabled(false);
	breakpoint_add_editor_.setTextToShowWhenEmpty("Add breakpoint...", Colours::grey);
	breakpoint_add_editor_.setInputRestrictions(4, "0123456789ABCDEF");
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

void CpuStatusComponent::OnCpuStateChanged(const CPU::Registers &registers, CPU::Flags flags)
{
	af_label_.setText(FormatRegisterLabelText("AF", registers.af), NotificationType::dontSendNotification);
	bc_label_.setText(FormatRegisterLabelText("BC", registers.bc), NotificationType::dontSendNotification);
	de_label_.setText(FormatRegisterLabelText("DE", registers.de), NotificationType::dontSendNotification);
	hl_label_.setText(FormatRegisterLabelText("HL", registers.hl), NotificationType::dontSendNotification);
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
	g.drawRect(breakpoint_list_box_.getBounds().expanded(1, 1), 1);   // draw an outline around the component
	g.drawRect(breakpoint_add_editor_.getBounds().expanded(1, 1), 1);   // draw an outline around the component
}

void CpuStatusComponent::resized()
{
	auto working_area = getLocalBounds();
	auto registers_area = working_area.removeFromTop(getHeight() / 4);
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

	auto breakpoint_list_area = working_area.removeFromTop(getHeight() / 2);
	breakpoint_add_editor_.setBounds(breakpoint_list_area.removeFromBottom(breakpoint_list_area.getHeight() / 8).reduced(1, 1));
	breakpoint_list_box_.setBounds(breakpoint_list_area.reduced(1, 0));
}

int CpuStatusComponent::getNumRows()
{
	return static_cast<int>(breakpoints_.size());
}

void CpuStatusComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/)
{
	if ((rowNumber < 0) || (rowNumber >= breakpoints_.size())) { return; }

	g.drawText(FormatRegisterLabelText("PC", breakpoints_[rowNumber]), 0, 0, width, height, Justification::centred);
}

void CpuStatusComponent::textEditorReturnKeyPressed(TextEditor &)
{
	const auto breakpoint = std::stoi(breakpoint_add_editor_.getText().toStdString(), 0, 16);
	if (breakpoint < std::numeric_limits<Memory::Address>::min() || breakpoint > std::numeric_limits<Memory::Address>::max()) return;

	breakpoint_add_editor_.clear();

	for (auto& listener : listeners_)
	{
		listener->OnBreakpointAdd(breakpoint);
	}
}
