#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <vector>
#include "JucyBoy/CPU.h"
#include "JucyBoy/MMU.h"
#include "JucyBoy/GPU.h"
#include "GameScreenComponent.h"
#include "CpuStatusComponent.h"
#include "MemoryMapComponent.h"

class JucyBoy final : public Component, public CPU::Listener, public AsyncUpdater
{
public:
	JucyBoy();
	~JucyBoy();

	void paint (Graphics&) override;
	void resized() override;

	void mouseDown(const MouseEvent &event) override;
	bool keyPressed(const KeyPress &key) override;

	// CPU::Listener overrides
	void OnRunningLoopInterrupted() override;

	// Transfers the handling of exception in running loop to the message thread
	void handleAsyncUpdate() override;

	// AddListener returns a deregister function that can be called with no arguments
	template <typename T>
	std::function<void()> AddListener(T &listener, void(T::*func)(bool))
	{
		auto it = listeners_.emplace(listeners_.begin(), std::bind(func, std::ref(listener), std::placeholders::_1));
		return [=, this]() { listeners_.erase(it); };
	}

private:
	void Reset();
	void LoadRom(const juce::File &file);

	// Listener notification
	void NotifyStatusUpdateRequest(bool compute_diff);

private:
	static const size_t cpu_status_width_{ 150 };
	static const size_t memory_map_width_{ 150 };

private:
	MMU mmu_{};
	CPU cpu_{ mmu_ };
	GPU gpu_{ mmu_ };

	std::vector<std::function<void()>> listener_deregister_functions_;

	GameScreenComponent game_screen_component_;
	Rectangle<int> usage_instructions_area_;
	CpuStatusComponent cpu_status_component_;
	MemoryMapComponent memory_map_component_;

	using Listener = std::function<void(bool)>;
	std::list<Listener> listeners_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
