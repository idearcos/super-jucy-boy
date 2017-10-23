#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "CPU/CpuRegistersComponent.h"
#include "CPU/CpuBreakpointsComponent.h"
#include "CPU/CpuInstructionBreakpointsComponent.h"
#include "Memory/MemoryMapComponent.h"
#include "Memory/WatchpointsComponent.h"
#include "../JucyBoy/Debug/DebugCPU.h"
#include "../JucyBoy/MMU.h"

class CpuDebugComponent final : public juce::Component
{
public:
	CpuDebugComponent();
	~CpuDebugComponent() = default;

	void SetCpu(DebugCPU* debug_cpu);
	void SetMmu(MMU* mmu) { memory_map_component_.SetMmu(mmu); }

	void OnEmulationStarted();
	void OnEmulationPaused();
	void UpdateState(bool compute_diff);

	void paint(juce::Graphics&) override;
	void resized() override;
	void visibilityChanged() override;

private:
	juce::Rectangle<int> usage_instructions_area_;
	CpuRegistersComponent registers_component_;
	CpuBreakpointsComponent breakpoints_component_;
	CpuInstructionBreakpointsComponent instruction_breakpoints_component_;
	MemoryMapComponent memory_map_component_;
	WatchpointsComponent watchpoints_component_;

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuDebugComponent)
};
