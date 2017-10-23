#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../JucyBoy/Debug/DebugCPU.h"

class CpuRegistersComponent final : public juce::Component, public juce::ListBoxModel
{
public:
	CpuRegistersComponent();
	~CpuRegistersComponent() = default;

	void SetCpu(DebugCPU* debug_cpu) { debug_cpu_ = debug_cpu; }

	// ListBoxModel overrides
	int getNumRows() override { return 6; }
	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

	void OnEmulationStarted();
	void OnEmulationPaused();
	void UpdateState(bool compute_diff);

	static juce::AttributedString FormatRegisterPairString(std::string register_name, const RegisterPair &register_pair_value, const RegisterPair &register_pair_previous_value, bool compute_difference, bool is_emulation_running);
	static juce::AttributedString Format16bitRegisterString(std::string register_name, uint16_t register_value, uint16_t register_previous_value, bool compute_difference, bool is_emulation_running);

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	juce::ListBox registers_list_;

	juce::ToggleButton carry_flag_toggle_;
	juce::ToggleButton half_carry_flag_toggle_;
	juce::ToggleButton subtract_flag_toggle_;
	juce::ToggleButton zero_flag_toggle_;

	CPU::Registers registers_state_;
	CPU::Registers previous_registers_state_;
	bool compute_diff_{ false };

	CPU::Flags previous_cpu_flags_state_;
	bool is_emulation_running_{ false };

	DebugCPU* debug_cpu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpuRegistersComponent)
};
