#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/CPU.h"
#include "JucyBoy/MMU.h"
#include "CpuStatusComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class JucyBoy final : public Component
{
public:
    //==============================================================================
	JucyBoy();
    ~JucyBoy();

    void paint (Graphics&) override;
    void resized() override;

	void mouseDown(const MouseEvent &event) override;
	bool keyPressed(const KeyPress &key) override;

private:
	void Reset();
	void LoadRom(const juce::File &file);

	void Run();
	void Stop();
	bool IsRunning() const;
	void RunningLoopFunction();

	void ExecuteNextInstruction();

private:
	CPU cpu_;
	MMU mmu_;

	bool is_running_;
	std::atomic<bool> exit_loop_;
	std::thread loop_function_thread_;

	CpuStatusComponent cpu_status_component_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
