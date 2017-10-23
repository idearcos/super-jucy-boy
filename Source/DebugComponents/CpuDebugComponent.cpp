#include "CpuDebugComponent.h"
#include <sstream>

CpuDebugComponent::CpuDebugComponent()
{
	addAndMakeVisible(registers_component_);
	addAndMakeVisible(breakpoints_component_);
	addAndMakeVisible(instruction_breakpoints_component_);
	addAndMakeVisible(memory_map_component_);
	addAndMakeVisible(watchpoints_component_);

	setSize(640, 480);
}

void CpuDebugComponent::SetCpu(DebugCPU* debug_cpu)
{
	// Clear previous listener interfaces
	if (debug_cpu_ != nullptr)
	{
		debug_cpu_->RemoveListener(breakpoints_component_);
		debug_cpu_->RemoveListener(instruction_breakpoints_component_);
		debug_cpu_->RemoveListener(watchpoints_component_);
	}

	registers_component_.SetCpu(debug_cpu);
	breakpoints_component_.SetCpu(debug_cpu);
	instruction_breakpoints_component_.SetCpu(debug_cpu);
	watchpoints_component_.SetCpu(debug_cpu);

	// Set listener interfaces
	if (debug_cpu != nullptr)
	{
		debug_cpu->AddListener(breakpoints_component_);
		debug_cpu->AddListener(instruction_breakpoints_component_);
		debug_cpu->AddListener(watchpoints_component_);
	}

	debug_cpu_ = debug_cpu;
}

void CpuDebugComponent::OnEmulationStarted()
{
	registers_component_.OnEmulationStarted();
	breakpoints_component_.OnEmulationStarted();
	instruction_breakpoints_component_.OnEmulationStarted();
	memory_map_component_.OnEmulationStarted();
	watchpoints_component_.OnEmulationStarted();
}

void CpuDebugComponent::OnEmulationPaused()
{
	registers_component_.OnEmulationPaused();
	breakpoints_component_.OnEmulationPaused();
	instruction_breakpoints_component_.OnEmulationPaused();
	memory_map_component_.OnEmulationPaused();
	watchpoints_component_.OnEmulationPaused();
}

void CpuDebugComponent::UpdateState(bool compute_diff)
{
	registers_component_.UpdateState(compute_diff);
	memory_map_component_.UpdateMemoryMap(compute_diff);
}

void CpuDebugComponent::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::white);

	g.setColour(juce::Colours::orange);
	g.drawRect(getLocalBounds(), 1);

	g.setFont(14.0f);

	std::stringstream usage_instructions;
	usage_instructions << "Space: run / stop" << std::endl;
	usage_instructions << "Right: step over" << std::endl;
	g.drawFittedText(usage_instructions.str(), usage_instructions_area_, juce::Justification::centred, 2);

	g.drawRect(usage_instructions_area_, 1);
}

void CpuDebugComponent::resized()
{
	auto working_area = getLocalBounds();
	auto cpu_debug_area = working_area.removeFromLeft(150);
	usage_instructions_area_ = cpu_debug_area.removeFromTop(40);
	registers_component_.setBounds(cpu_debug_area.removeFromTop(110));
	breakpoints_component_.setBounds(cpu_debug_area.removeFromTop(cpu_debug_area.getHeight() / 2));
	instruction_breakpoints_component_.setBounds(cpu_debug_area);

	memory_map_component_.setBounds(working_area.removeFromTop(4 * working_area.getHeight() / 5));
	watchpoints_component_.setBounds(working_area);
}

void CpuDebugComponent::visibilityChanged()
{
	// When closing the CPU debugger component, remove all interfaces with JucyBoy
	if (!isVisible())
	{
		SetCpu(nullptr);
		SetMmu(nullptr);
	}
}
