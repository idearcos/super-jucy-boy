#include "CpuRegistersComponent.h"
#include <sstream>
#include <iomanip>

CpuRegistersComponent::CpuRegistersComponent()
{
	// Add registers listbox
	registers_list_.setModel(this);
	addAndMakeVisible(registers_list_);

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

	UpdateState(false);
}

void CpuRegistersComponent::paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool /*rowIsSelected*/)
{
	switch (rowNumber)
	{
	case 0:
		FormatRegisterPairString("AF", registers_state_.af, previous_registers_state_.af, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	case 1:
		FormatRegisterPairString("BC", registers_state_.bc, previous_registers_state_.bc, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	case 2:
		FormatRegisterPairString("DE", registers_state_.de, previous_registers_state_.de, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	case 3:
		FormatRegisterPairString("HL", registers_state_.hl, previous_registers_state_.hl, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	case 4:
		Format16bitRegisterString("SP", registers_state_.sp, previous_registers_state_.sp, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	case 5:
		Format16bitRegisterString("PC", registers_state_.pc, previous_registers_state_.pc, compute_diff_, is_emulation_running_).draw(g, Rectangle<int>{ width, height }.toFloat());
		break;
	}
}

AttributedString CpuRegistersComponent::FormatRegisterPairString(std::string register_name, const RegisterPair &register_pair_value, const RegisterPair &register_pair_previous_value, bool compute_diff, bool is_emulation_running)
{
	AttributedString formatted_string;
	formatted_string.append(register_name);
	formatted_string.append(": ");

	{std::stringstream high;
	high << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(register_pair_value.GetHigh());
	const auto color = is_emulation_running ? Colours::grey
		: ((compute_diff && (register_pair_value.GetHigh() != register_pair_previous_value.GetHigh())) ? Colours::red
			: Colours::black);
	formatted_string.append(high.str(), color); }

	{std::stringstream low;
	low << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(register_pair_value.GetLow());
	const auto color = is_emulation_running ? Colours::grey
		: ((compute_diff && (register_pair_value.GetLow() != register_pair_previous_value.GetLow())) ? Colours::red
			: Colours::black);
	formatted_string.append(low.str(), color); }

	formatted_string.setJustification(Justification::centred);
	formatted_string.setFont(Font{ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain });

	return formatted_string;
}

AttributedString CpuRegistersComponent::Format16bitRegisterString(std::string register_name, uint16_t register_value, uint16_t register_previous_value, bool compute_diff, bool is_emulation_running)
{
	AttributedString formatted_string;
	formatted_string.append(register_name);
	formatted_string.append(": ");

	std::stringstream value;
	value << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << register_value;
	const auto color = is_emulation_running ? Colours::grey
		: ((compute_diff && (register_value != register_previous_value)) ? Colours::red
			: Colours::black);
	formatted_string.append(value.str(), color);

	formatted_string.setJustification(Justification::centred);
	formatted_string.setFont(Font{ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain });

	return formatted_string;
}

void CpuRegistersComponent::OnEmulationStarted()
{
	is_emulation_running_ = true;
	carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::grey);
	half_carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::grey);
	subtract_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::grey);
	zero_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::grey);
	repaint();
}

void CpuRegistersComponent::OnEmulationPaused()
{
	is_emulation_running_ = false;
	carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
	half_carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
	subtract_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
	zero_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, Colours::black);
	repaint();
}

void CpuRegistersComponent::UpdateState(bool compute_diff)
{
	if (!debug_cpu_) return;

	compute_diff_ = compute_diff;

	previous_registers_state_ = registers_state_;
	registers_state_ = debug_cpu_->GetRegistersState();
	const auto flags_state = debug_cpu_->GetFlagsState();

	carry_flag_toggle_.setToggleState((flags_state & CPU::Flags::C) != CPU::Flags::None, NotificationType::dontSendNotification);
	half_carry_flag_toggle_.setToggleState((flags_state & CPU::Flags::H) != CPU::Flags::None, NotificationType::dontSendNotification);
	subtract_flag_toggle_.setToggleState((flags_state & CPU::Flags::N) != CPU::Flags::None, NotificationType::dontSendNotification);
	zero_flag_toggle_.setToggleState((flags_state & CPU::Flags::Z) != CPU::Flags::None, NotificationType::dontSendNotification);

	carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId,  compute_diff && ((flags_state & CPU::Flags::C) != (previous_cpu_flags_state_ & CPU::Flags::C)) ? Colours::red : Colours::black);
	half_carry_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, compute_diff && ((flags_state & CPU::Flags::H) != (previous_cpu_flags_state_ & CPU::Flags::H)) ? Colours::red : Colours::black);
	subtract_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, compute_diff && ((flags_state & CPU::Flags::N) != (previous_cpu_flags_state_ & CPU::Flags::N)) ? Colours::red : Colours::black);
	zero_flag_toggle_.setColour(ToggleButton::ColourIds::textColourId, compute_diff && ((flags_state & CPU::Flags::Z) != (previous_cpu_flags_state_ & CPU::Flags::Z)) ? Colours::red : Colours::black);

	previous_cpu_flags_state_ = flags_state;

	repaint();
}

void CpuRegistersComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuRegistersComponent::resized()
{
	auto working_area = getLocalBounds();
	auto flags_area = working_area.removeFromRight(getWidth() / 3);

	registers_list_.setBounds(working_area.reduced(1, 1));
	registers_list_.setRowHeight(working_area.getHeight() / 6);

	const auto flags_area_height = flags_area.getHeight();
	zero_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	subtract_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	half_carry_flag_toggle_.setBounds(flags_area.removeFromTop(flags_area_height / 4));
	carry_flag_toggle_.setBounds(flags_area);
}
