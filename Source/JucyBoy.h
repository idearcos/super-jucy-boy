#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <vector>
#include "JucyBoy/CPU.h"
#include "JucyBoy/MMU.h"
#include "JucyBoy/GPU.h"
#include "CpuStatusComponent.h"
#include "GameScreenComponent.h"

class JucyBoy final : public Component, public CPU::Listener, public AsyncUpdater, public CpuStatusComponent::Listener
{
public:
	JucyBoy();
	~JucyBoy();

	void paint (Graphics&) override;
	void resized() override;

	void mouseDown(const MouseEvent &event) override;
	bool keyPressed(const KeyPress &key) override;

	// CPU::Listener overrides
	void OnCpuStateChanged(const CPU::Registers &/*registers*/, CPU::Flags /*flags*/) override {}
	void OnBreakpointsChanged(const CPU::BreakpointList &/*breakpoint_list*/) override {}
	void OnRunningLoopExited() override;
	void OnCyclesLapsed(CPU::MachineCycles /*cycles*/) override {}

	// CpuStatusComponent::Listener overrides
	void OnBreakpointAdd(Memory::Address breakpoint) override;

	// Transfers the handling of exception in running loop to the message thread
	void handleAsyncUpdate() override;

private:
	void Reset();
	void LoadRom(const juce::File &file);

private:
	MMU mmu_{};
	CPU cpu_{ mmu_ };
	GPU gpu_{ mmu_ };

	std::vector<std::function<void()>> listener_deregister_functions_;

	CpuStatusComponent cpu_status_component_;
	GameScreenComponent game_screen_component_;

	// Removal of listeners
	MMU::ListenerIterator gpu_io_it_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
