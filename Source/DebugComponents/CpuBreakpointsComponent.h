#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Debug/DebugCPU.h"

class CpuBreakpointsComponent final : public Component, public ListBoxModel, public TextEditor::Listener, public DebugCPU::Listener
{
public:
	CpuBreakpointsComponent(DebugCPU& debug_cpu);
	~CpuBreakpointsComponent() = default;

	void OnEmulationStarted();
	void OnEmulationPaused();

	// DebugCPU::Listener overrides
	void OnBreakpointHit(Memory::Address breakpoint) override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(TextEditor&) override;

	// Component overrides
	void paint(Graphics& g) override;
	void resized() override;

private:
	void UpdateBreakpoints();

private:
	std::vector<Memory::Address> breakpoints_;

	Label breakpoint_list_header_;
	ListBox breakpoint_list_box_;

	TextEditor breakpoint_add_editor_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuBreakpointsComponent)
};
