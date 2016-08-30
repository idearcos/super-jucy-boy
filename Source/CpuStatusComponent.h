#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"

class CPU;

//==============================================================================
/*
*/
class CpuStatusComponent : public Component, public CPU::Listener
{
public:
	CpuStatusComponent();
	~CpuStatusComponent();

	void paint (Graphics&) override;
	void resized() override;

	void OnCpuStateChanged(const CPU::Registers &registers, CPU::Flags flags) override;

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuStatusComponent)
};
