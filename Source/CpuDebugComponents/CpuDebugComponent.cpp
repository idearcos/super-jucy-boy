#include "CpuDebugComponent.h"

CpuDebugComponent::CpuDebugComponent(CPU &cpu) :
	breakpoints_component_{ cpu },
	instruction_breakpoints_component_{ cpu },
	cpu_{ &cpu }
{
	addAndMakeVisible(registers_component_);
	addAndMakeVisible(breakpoints_component_);
	addAndMakeVisible(instruction_breakpoints_component_);
}

void CpuDebugComponent::OnStatusUpdateRequested(bool compute_diff)
{
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
	breakpoints_component_.setBounds(working_area.removeFromTop(working_area.getHeight() / 2));
	instruction_breakpoints_component_.setBounds(working_area);
}
