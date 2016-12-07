#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "JucyBoy/Memory.h"

class MMU;

class MemoryMapComponent : public Component, public ListBoxModel
{
public:
	MemoryMapComponent(MMU &mmu);
	~MemoryMapComponent();

	// JucyBoy Listener functions
	void OnStatusUpdateRequested(bool compute_diff);

	void paint (Graphics&) override;
	void resized() override;

	// ListBoxModel overrides
	int getNumRows() override;
	void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

private:
	static std::string FormatListBoxRowText(size_t memory_address, uint8_t value);

private:
	Memory::Map memory_map_{};
	ListBox memory_map_list_box_;
	Label memory_map_list_header_;

	Memory::Map previous_memory_map_state_{};
	std::vector<Colour> memory_map_colours_{ memory_map_.size(), Colours::black };

	MMU* mmu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMapComponent)
};
