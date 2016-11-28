#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"

class CpuStatusComponent final : public Component, public ListBoxModel, public TextEditor::Listener, public CPU::Listener
{
public:
	CpuStatusComponent(CPU &cpu);
	~CpuStatusComponent();

	// JucyBoy Listener functions
	void OnStatusUpdateRequested(bool compute_diff);

	// CPU::Listener overrides
	void OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list) override;

	void paint(Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(TextEditor &) override;

private:
	static std::string FormatRegisterLabelText(std::string register_name, uint16_t value);

private:
	Label af_label_;
	Label bc_label_;
	Label de_label_;
	Label hl_label_;
	Label sp_label_;
	Label pc_label_;

	ToggleButton carry_flag_toggle_;
	ToggleButton half_carry_flag_toggle_;
	ToggleButton subtract_flag_toggle_;
	ToggleButton zero_flag_toggle_;

	CPU::Registers previous_registers_state_;
	CPU::Flags previous_cpu_flags_state_;

	std::vector<Memory::Address> breakpoints_;
	Label breakpoint_list_header_;
	ListBox breakpoint_list_box_;
	TextEditor breakpoint_add_editor_;

	CPU* cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuStatusComponent)
};
