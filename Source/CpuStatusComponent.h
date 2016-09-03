#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"

class CPU;

//==============================================================================
/*
*/
class CpuStatusComponent final : public Component, public CPU::Listener, public ListBoxModel
{
public:
	CpuStatusComponent();
	~CpuStatusComponent();

	void OnCpuStateChanged(const CPU::Registers &registers, CPU::Flags flags) override;
	void OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list) override;
	void OnRunningLoopExited() override {};

	void paint(Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

private:
	static std::string FormatRegisterLabelText(std::string register_name, uint16_t value);

private:
	Label af_label_;
	Label bc_label_;
	Label de_label_;
	Label hl_label_;
	Label pc_label_;
	Label sp_label_;

	ToggleButton carry_flag_toggle_;
	ToggleButton half_carry_flag_toggle_;
	ToggleButton subtract_flag_toggle_;
	ToggleButton zero_flag_toggle_;

	std::vector<uint16_t> breakpoints_;
	ListBox breakpoint_list_box_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuStatusComponent)
};
