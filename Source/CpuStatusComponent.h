#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class CPU;

//==============================================================================
/*
*/
class CpuStatusComponent    : public Component
{
public:
    CpuStatusComponent();
    ~CpuStatusComponent();

    void paint (Graphics&) override;
    void resized() override;

	void SetCpuState(const CPU &cpu);

private:
	uint8_t a_;
	uint8_t f_;
	uint8_t b_;
	uint8_t c_;
	uint8_t d_;
	uint8_t e_;
	uint8_t h_;
	uint8_t l_;
	uint16_t pc_;
	uint16_t sp_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuStatusComponent)
};
