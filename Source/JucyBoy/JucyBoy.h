#pragma once

#include "CPU.h"
#include "MMU.h"

class JucyBoy final
{
public:
	JucyBoy();
	~JucyBoy();

	void Run();
	void Stop();

private:
	void RunningLoopFunction();

private:
	CPU cpu_;
	MMU mmu_;

	std::atomic<bool> exit_loop_;
	std::thread loop_function_thread_;
};
