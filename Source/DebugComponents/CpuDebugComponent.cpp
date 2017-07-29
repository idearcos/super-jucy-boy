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
	registers_component_.SetCpu(debug_cpu);
	breakpoints_component_.SetCpu(debug_cpu);
	instruction_breakpoints_component_.SetCpu(debug_cpu);
	watchpoints_component_.SetCpu(debug_cpu);
}

void CpuDebugComponent::OnEmulationStarted()
{
	registers_component_.OnEmulationStarted();
	breakpoints_component_.OnEmulationStarted();
	instruction_breakpoints_component_.OnEmulationStarted();
	watchpoints_component_.OnEmulationStarted();
}

void CpuDebugComponent::OnEmulationPaused()
{
	registers_component_.OnEmulationPaused();
	breakpoints_component_.OnEmulationPaused();
	instruction_breakpoints_component_.OnEmulationPaused();
	watchpoints_component_.OnEmulationPaused();
}

void CpuDebugComponent::UpdateState(bool compute_diff)
{
	registers_component_.UpdateState(compute_diff);
}

void CpuDebugComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
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
