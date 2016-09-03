#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"
#include "JucyBoy/MMU.h"
#include "CpuStatusComponent.h"

class JucyBoy final : public Component, public CPU::Listener, public AsyncUpdater
{
public:
	JucyBoy();
	~JucyBoy();

	void paint (Graphics&) override;
	void resized() override;

	void mouseDown(const MouseEvent &event) override;
	bool keyPressed(const KeyPress &key) override;

	void OnCpuStateChanged(const CPU::Registers &/*registers*/, CPU::Flags /*flags*/) override {}
	void OnBreakpointsChanged(const CPU::BreakpointList &/*breakpoint_list*/) override {}
	void OnRunningLoopExited() override;

	// Transfers the handling of exception in running loop to the message thread
	void handleAsyncUpdate() override;

private:
	void Reset();
	void LoadRom(const juce::File &file);

private:
	MMU mmu_{};
	CPU cpu_{ mmu_ };

	CpuStatusComponent cpu_status_component_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
