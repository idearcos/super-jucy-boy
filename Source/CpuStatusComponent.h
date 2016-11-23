#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"

class CpuStatusComponent final : public Component, public CPU::Listener, public ListBoxModel, public TextEditor::Listener
{
public:
	CpuStatusComponent();
	~CpuStatusComponent();

	void OnCpuStateChanged(const CPU::Registers &registers, CPU::Flags flags) override;
	void OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list) override;
	void OnRunningLoopExited() override {}
	void OnCyclesLapsed(CPU::MachineCycles /*cycles*/) override {}

	void paint(Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(TextEditor &) override;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnBreakpointAdd(Memory::Address breakpoint) = 0;
	};

	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

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

	std::vector<Memory::Address> breakpoints_;
	ListBox breakpoint_list_box_;
	TextEditor breakpoint_add_editor_;

	std::set<Listener*> listeners_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuStatusComponent)
};
