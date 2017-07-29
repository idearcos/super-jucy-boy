#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Debug/DebugCPU.h"
#include <set>

class CpuInstructionBreakpointsComponent final : public Component, public ListBoxModel, public ComboBox::Listener, public Button::Listener, public DebugCPU::Listener
{
public:
	CpuInstructionBreakpointsComponent();
	~CpuInstructionBreakpointsComponent() = default;

	void SetCpu(DebugCPU& debug_cpu);

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

	// Component overrides
	void paint(Graphics&) override;
	void resized() override;

private:
	std::set<CPU::OpCode> instruction_breakpoints_;

	Label instruction_breakpoint_list_header_;
	ListBox instruction_breakpoint_list_box_;

	ComboBox instruction_breakpoint_add_combo_box_;
	TextButton instruction_breakpoint_add_button_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuInstructionBreakpointsComponent)
};
