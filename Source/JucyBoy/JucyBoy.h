#pragma once

#include <string>
#include "Debug/DebugCPU.h"
#include "MMU.h"
#include "PPU.h"
#include "APU.h"
#include "Timer.h"
#include "Joypad.h"
#include "Cartridge.h"

class JucyBoy
{
public:
	JucyBoy(const std::string &rom_file_path);
	~JucyBoy() = default;

	void StartEmulation(bool debug);
	void PauseEmulation();
	inline bool IsRunning() const { return cpu_.IsRunning(); }

	void StepOver(bool debug);

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(cpu_, mmu_, ppu_, apu_, timer_, joypad_, cartridge_);
	}

public:
	DebugCPU& GetCpu() { return cpu_; }
	MMU& GetMmu() { return mmu_; }
	PPU& GetPpu() { return ppu_; }
	APU& GetApu() { return apu_; }
	Joypad& GetJoypad() { return joypad_; }

private:
	MMU mmu_;
	DebugCPU cpu_{ mmu_ };
	PPU ppu_{ mmu_ };
	APU apu_;
	Timer timer_{ mmu_ };
	Joypad joypad_;
	Cartridge cartridge_;
};
