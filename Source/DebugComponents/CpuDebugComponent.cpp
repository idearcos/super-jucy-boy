#include "CpuDebugComponent.h"

CpuDebugComponent::CpuDebugComponent()
{
	addAndMakeVisible(registers_component_);
	addAndMakeVisible(breakpoints_component_);
	addAndMakeVisible(instruction_breakpoints_component_);
	addAndMakeVisible(watchpoints_component_);
}

void CpuDebugComponent::SetCpu(DebugCPU& debug_cpu)
{
	cpu_ = &debug_cpu;
	breakpoints_component_.SetCpu(debug_cpu);
	instruction_breakpoints_component_.SetCpu(debug_cpu);
	watchpoints_component_.SetCpu(debug_cpu);
}

void CpuDebugComponent::UpdateStatus(bool compute_diff)
{
	if (!cpu_) return;

	registers_component_.UpdateRegistersState(cpu_->GetRegistersState(), cpu_->GetFlagsState(), compute_diff);
	breakpoints_component_.UpdateHitBreakpoint(cpu_->GetRegistersState().pc);
}

void CpuDebugComponent::paint(Graphics& g)
{
	g.fillAll(Colours::white);

	g.setColour(Colours::orange);
	g.drawRect(getLocalBounds(), 1);
}

void CpuDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	registers_component_.setBounds(working_area.removeFromTop(110));
	const auto breakpoints_area_height = working_area.getHeight();
	breakpoints_component_.setBounds(working_area.removeFromTop(breakpoints_area_height / 3));
	instruction_breakpoints_component_.setBounds(working_area.removeFromTop(breakpoints_area_height / 3));
	watchpoints_component_.setBounds(working_area);
}
