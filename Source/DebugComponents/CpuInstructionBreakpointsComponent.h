#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Debug/DebugCPU.h"

class CpuInstructionBreakpointsComponent final : public Component, public ListBoxModel, public ComboBox::Listener, public Button::Listener, public DebugCPU::Listener
{
public:
	CpuInstructionBreakpointsComponent(DebugCPU& debug_cpu);
	~CpuInstructionBreakpointsComponent() = default;

	void OnEmulationStarted();
	void OnEmulationPaused();

	// DebugCPU::Listener overrides
	void OnInstructionBreakpointHit(CPU::OpCode opcode) override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// ComboBox::Listener overrides
	void comboBoxChanged(ComboBox*) override {}

	// Button::Listener overrides
	void buttonClicked(Button*) override;

	void UpdateInstructionBreakpoints();

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

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuInstructionBreakpointsComponent)
};
