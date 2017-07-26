#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <functional>
#include <vector>
#include "JucyBoy/Debug/DebugCPU.h"
#include "JucyBoy/MMU.h"
#include "JucyBoy/Debug/DebugPPU.h"
#include "JucyBoy/APU.h"
#include "JucyBoy/Timer.h"
#include "JucyBoy/Joypad.h"
#include "JucyBoy/Cartridge.h"
#include "GameScreenComponent.h"
#include "AudioPlayerComponent.h"
#include "DebugComponents/CpuDebugComponent.h"
#include "DebugComponents/MemoryMapComponent.h"
#include "DebugComponents/PpuDebugComponent.h"

class JucyBoy final : public Component, public CPU::Listener, public DebugCPU::Listener
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

private:
	void LoadRom(std::string file_path);
	void StartEmulation();
	void PauseEmulation();
	void UpdateDebugComponents(bool compute_diff);

	void ConstructDebugComponents();

	// Save/load state
	void SaveState() const;
	void LoadState();

	// Toggle GUI features on/off
	void EnableDebugging(Component &component, bool enable);
	int ComputeWindowWidth() const;

private:
	static const size_t cpu_status_width_{ 150 };
	static const size_t memory_map_width_{ 430 };
	static const size_t ppu_tileset_width_{ 128 * 2 };

private:
	std::unique_ptr<DebugCPU> cpu_;
	std::unique_ptr<MMU> mmu_;
	std::unique_ptr<DebugPPU> ppu_;
	std::unique_ptr<APU> apu_;
	std::unique_ptr<jb::Timer> timer_;
	std::unique_ptr<Joypad> joypad_;
	std::unique_ptr<Cartridge> cartridge_;

	std::vector<std::function<void()>> listener_deregister_functions_;

	std::string loaded_rom_file_path_;

	GameScreenComponent game_screen_component_;
	AudioPlayerComponent audio_player_component_;

	Rectangle<int> usage_instructions_area_;
	std::unique_ptr<CpuDebugComponent> cpu_debug_component_;
	std::unique_ptr<MemoryMapComponent> memory_map_component_;
	std::unique_ptr<PpuDebugComponent> ppu_debug_component_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucyBoy)
};
