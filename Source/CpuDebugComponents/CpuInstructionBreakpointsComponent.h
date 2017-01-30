#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/CPU.h"

class CpuInstructionBreakpointsComponent final : public Component, public ListBoxModel, public ComboBox::Listener, public Button::Listener, public CPU::Listener
{
public:
	CpuInstructionBreakpointsComponent(CPU &cpu);
	~CpuInstructionBreakpointsComponent() = default;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// ComboBox::Listener overrides
	void comboBoxChanged(ComboBox*) override {}

	// Button::Listener overrides
	void buttonClicked(Button*) override;

	// CPU::Listener overrides
	void OnInstructionBreakpointsChanged(const CPU::InstructionBreakpointList &instruction_breakpoint_list) override;

	// Component overrides
	void paint(Graphics&) override;
	void resized() override;

	void UpdateHitInstructionBreakpoint(CPU::OpCode opcode);

private:
	std::vector<CPU::OpCode> instruction_breakpoints_;

	Label instruction_breakpoint_list_header_;
	ListBox instruction_breakpoint_list_box_;

	ComboBox instruction_breakpoint_add_combo_box_;
	TextButton instruction_breakpoint_add_button_;

	CPU* cpu_{ nullptr };
};
