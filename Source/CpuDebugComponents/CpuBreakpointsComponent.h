#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/CPU.h"

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
	void textEditorReturnKeyPressed(TextEditor&) override;

	// CPU::Listener overrides
	void OnBreakpointsChanged(const CPU::BreakpointList &breakpoint_list) override;

	// Component overrides
	void paint(Graphics& g) override;
	void resized() override;

	void UpdateHitBreakpoint(Memory::Address pc);

private:
	std::vector<Memory::Address> breakpoints_;

	Label breakpoint_list_header_;
	ListBox breakpoint_list_box_;

	TextEditor breakpoint_add_editor_;

	CPU* cpu_{ nullptr };
};
