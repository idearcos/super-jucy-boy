#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "CpuRegistersComponent.h"
#include "CpuBreakpointsComponent.h"
#include "CpuInstructionBreakpointsComponent.h"
#include "../JucyBoy/CPU.h"

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
	CpuRegistersComponent registers_component_;
	CpuBreakpointsComponent breakpoints_component_;
	CpuInstructionBreakpointsComponent instruction_breakpoints_component_;

	CPU* cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuDebugComponent)
};