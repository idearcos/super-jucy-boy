#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Memory.h"
#include "../JucyBoy/Debug/DebugCPU.h"
#include <set>

class WatchpointsComponent final : public juce::Component, public juce::ListBoxModel, public juce::TextEditor::Listener, public DebugCPU::Listener
{
public:
	WatchpointsComponent();
	~WatchpointsComponent() = default;

	void SetCpu(DebugCPU* debug_cpu);

	void OnEmulationStarted();
	void OnEmulationPaused();

	// DebugCPU::Listener overrides
	void OnWatchpointHit(Memory::Watchpoint watchpoint) override;

	void paint(juce::Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(juce::TextEditor &) override;

private:
	std::set<Memory::Watchpoint> watchpoints_;
	juce::Label watchpoint_list_header_;
	juce::ListBox watchpoint_list_box_;

	juce::TextEditor watchpoint_add_editor_;
	juce::ToggleButton watchpoint_type_read_{ "Read" };
	juce::ToggleButton watchpoint_type_write_{ "Write" };
	juce::Rectangle<int> watchpoint_add_area_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WatchpointsComponent)
};
