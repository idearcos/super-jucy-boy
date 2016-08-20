#include "JucyBoy.h"

JucyBoy::JucyBoy() :
	cpu_{ mmu_ },
	exit_loop_{ false }
{

}

JucyBoy::~JucyBoy()
{
	Stop();
}

void JucyBoy::RunningLoopFunction()
{
	while (!exit_loop_.load())
	{
		auto cycles = cpu_.ExecuteInstruction(cpu_.FetchOpcode());
	}
}

void JucyBoy::Run()
{
	if (loop_function_thread_.joinable()) { return; }

	exit_loop_.store(false);
	loop_function_thread_ = std::thread{ &JucyBoy::RunningLoopFunction, this };
}

void JucyBoy::Stop()
{
	if (!loop_function_thread_.joinable()) { return; }

	exit_loop_.store(true);
	loop_function_thread_.join();
}
