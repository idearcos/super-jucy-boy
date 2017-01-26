#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"

class CpuRegistersStateComponent final : public Component
{
public:
	CpuRegistersStateComponent();
	~CpuRegistersStateComponent() = default;

	void UpdateRegistersState(CPU::Registers registers_state, CPU::Flags flags_state, bool compute_diff);

	static std::string FormatRegisterLabelText(std::string register_name, uint16_t value);

	void paint(Graphics&) override;
	void resized() override;

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
};

class CpuBreakpointsComponent final : public Component, public ListBoxModel, public TextEditor::Listener, public CPU::Listener
{
public:
	CpuBreakpointsComponent(CPU &cpu);
	~CpuBreakpointsComponent();

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(TextEditor &) override;

	// CPU::Listener overrides
	void OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list) override;

	void UpdateHitBreakpoint(Memory::Address pc);

	void paint(Graphics&) override;
	void resized() override;

private:
	std::vector<Memory::Address> breakpoints_;
	Label breakpoint_list_header_;
	ListBox breakpoint_list_box_;
	TextEditor breakpoint_add_editor_;

	CPU* cpu_{ nullptr };
};

class CpuDebugComponent final : public Component
{
public:
	CpuDebugComponent(CPU &cpu);
	~CpuDebugComponent() = default;

	// JucyBoy Listener functions
	void OnStatusUpdateRequested(bool compute_diff);

	void paint(Graphics&) override;
	void resized() override;

private:
	CpuRegistersStateComponent registers_state_component_;
	CpuBreakpointsComponent breakpoints_component_;

	CPU* cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuDebugComponent)
};
