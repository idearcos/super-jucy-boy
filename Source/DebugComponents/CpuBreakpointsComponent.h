#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Debug/DebugCPU.h"

class CpuBreakpointsComponent final : public juce::Component, public juce::ListBoxModel, public juce::TextEditor::Listener, public DebugCPU::Listener
{
public:
	CpuBreakpointsComponent();
	~CpuBreakpointsComponent() = default;

	void SetCpu(DebugCPU* debug_cpu);

	void OnEmulationStarted();
	void OnEmulationPaused();

	// DebugCPU::Listener overrides
	void OnBreakpointHit(Memory::Address breakpoint) override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(juce::TextEditor&) override;

	// Component overrides
	void paint(juce::Graphics& g) override;
	void resized() override;

private:
	std::set<Memory::Address> breakpoints_;

	juce::Label breakpoint_list_header_;
	juce::ListBox breakpoint_list_box_;

	juce::TextEditor breakpoint_add_editor_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuBreakpointsComponent)
};
