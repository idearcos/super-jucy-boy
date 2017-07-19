#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Memory.h"
#include <vector>

class MMU;

class MemoryMapComponent final : public Component, public ListBoxModel
{
public:
	MemoryMapComponent(MMU &mmu);
	~MemoryMapComponent() = default;

	void OnEmulationStarted();
	void OnEmulationPaused();
	void UpdateState(bool compute_diff);

	// Component overrides
	void paint(Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

private:
	Memory::Map memory_map_{};

	Label memory_map_list_header_;
	ListBox memory_map_list_box_;

	Memory::Map previous_memory_map_state_{};
	std::vector<Colour> memory_map_colours_{ memory_map_.size(), Colours::black };
	bool is_emulation_running_{ false };

	MMU* mmu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemoryMapComponent)
};
