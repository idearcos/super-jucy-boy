#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../JucyBoy/Memory.h"
#include <vector>

class MMU;

class MemoryMapComponent final : public juce::Component, public juce::ListBoxModel
{
public:
	MemoryMapComponent();
	~MemoryMapComponent() = default;

	void SetMmu(MMU* mmu) { mmu_ = mmu; }

	void OnEmulationStarted();
	void OnEmulationPaused();
	void UpdateMemoryMap(bool compute_diff);

	// Component overrides
	void paint(juce::Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
	Memory::Map memory_map_{};

	juce::Label memory_map_list_header_;
	juce::ListBox memory_map_list_box_;

	Memory::Map previous_memory_map_state_{};
	std::vector<juce::Colour> memory_map_colours_{ memory_map_.size(), juce::Colours::black };
	bool is_emulation_running_{ false };

	MMU* mmu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemoryMapComponent)
};
