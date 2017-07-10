#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "CpuRegistersComponent.h"
#include "CpuBreakpointsComponent.h"
#include "CpuInstructionBreakpointsComponent.h"
#include "WatchpointsComponent.h"
#include "../JucyBoy/Debug/DebugCPU.h"

class CpuDebugComponent final : public Component
{
public:
	CpuDebugComponent();
	~CpuDebugComponent() = default;

	void SetCpu(DebugCPU& cpu);

	void UpdateStatus(bool compute_diff);

	void paint(Graphics&) override;
	void resized() override;

private:
	CpuRegistersComponent registers_component_;
	CpuBreakpointsComponent breakpoints_component_;
	CpuInstructionBreakpointsComponent instruction_breakpoints_component_;
	WatchpointsComponent watchpoints_component_;

	DebugCPU* cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuDebugComponent)
};
