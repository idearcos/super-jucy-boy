#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Memory.h"
#include "../JucyBoy/Debug/DebugCPU.h"
#include <vector>

class WatchpointsComponent final : public Component, public ListBoxModel, public TextEditor::Listener, public DebugCPU::Listener
{
public:
	WatchpointsComponent(DebugCPU& debug_cpu);
	~WatchpointsComponent() = default;

	void OnEmulationStarted();
	void OnEmulationPaused();

	// DebugCPU::Listener overrides
	void OnWatchpointHit(Memory::Watchpoint watchpoint) override;

	void paint(Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
	void deleteKeyPressed(int lastRowSelected) override;

	// TextEditor::Listener overrides
	void textEditorReturnKeyPressed(TextEditor &) override;

private:
	std::vector<Memory::Watchpoint> watchpoints_;
	Label watchpoint_list_header_;
	ListBox watchpoint_list_box_;

	TextEditor watchpoint_add_editor_;
	ToggleButton watchpoint_type_read_{ "Read" };
	ToggleButton watchpoint_type_write_{ "Write" };
	Rectangle<int> watchpoint_add_area_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WatchpointsComponent)
};
