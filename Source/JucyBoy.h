#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <vector>
#include "JucyBoy/Debug/DebugCPU.h"
#include "JucyBoy/Debug/DebugMMU.h"
#include "JucyBoy/Debug/DebugPPU.h"
#include "JucyBoy/APU.h"
#include "JucyBoy/Timer.h"
#include "JucyBoy/Joypad.h"
#include "JucyBoy/Cartridge.h"
#include "GameScreenComponent.h"
#include "AudioPlayerComponent.h"
#include "DebugComponents/CpuDebugComponent.h"
#include "DebugComponents/MemoryDebugComponent.h"
#include "DebugComponents/PpuDebugComponent.h"

class JucyBoy final : public Component, public CPU::Listener, public AsyncUpdater
{
public:
	JucyBoy();
	~JucyBoy();

	void paint (Graphics&) override;
	void resized() override;

	void mouseDown(const MouseEvent &event) override;
	bool keyPressed(const KeyPress &key) override;
	bool keyStateChanged(bool isKeyDown) override;

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
	static const size_t memory_map_width_{ 430 };
	static const size_t ppu_tileset_width_{ 128 * 2 };

private:
	DebugMMU mmu_{};
	DebugCPU cpu_{ mmu_ };
	DebugPPU ppu_{ mmu_ };
	APU apu_;
	jb::Timer timer_{ mmu_ };
	Joypad joypad_;
	std::unique_ptr<Cartridge> cartridge_;

	std::vector<std::function<void()>> listener_deregister_functions_;

	GameScreenComponent game_screen_component_;
	AudioPlayerComponent audio_player_component_;

	Rectangle<int> usage_instructions_area_;
	CpuDebugComponent cpu_debug_component_{ cpu_ };
	MemoryDebugComponent memory_debug_component_{ mmu_ };
	PpuDebugComponent ppu_debug_component_{ ppu_ };

	using Listener = std::function<void(bool)>;
	std::list<Listener> listeners_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
