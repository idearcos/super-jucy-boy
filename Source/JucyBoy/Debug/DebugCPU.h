#pragma once

#include "../CPU.h"
#include <set>

class MMU;

class DebugCPU final : public CPU
{
public:
	using BreakpointList = std::set<Memory::Address>;
	using InstructionBreakpointList = std::set<OpCode>;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnBreakpointHit(Memory::Address/*breakpoint*/) {}
		virtual void OnInstructionBreakpointHit(OpCode /*opcode*/) {}
		virtual void OnWatchpointHit(Memory::Watchpoint /*watchpoint*/) {}
	};

	DebugCPU(MMU &mmu);
	~DebugCPU() = default;

	void DebugRun();
	void DebugStepOver();

	// Status retrieval
	inline Registers GetRegistersState() const { return registers_; }
	inline Flags GetFlagsState() const { return ReadFlags(); }

	// Breakpoints
	inline void AddBreakpoint(Memory::Address address) { breakpoints_.insert(address); } //TODO: allow only when not running!
	inline void RemoveBreakpoint(Memory::Address address) { breakpoints_.erase(address); } //TODO: allow only when not running!
	inline void AddInstructionBreakpoint(OpCode opcode) { instruction_breakpoints_.insert(opcode); } //TODO: allow only when not running!
	inline void RemoveInstructionBreakpoint(OpCode opcode) { instruction_breakpoints_.erase(opcode); } //TODO: allow only when not running!

	// Watchpoints
	void AddWatchpoint(Memory::Watchpoint watchpoint);
	void RemoveWatchpoint(Memory::Watchpoint watchpoint);
	
	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	void DebugRunningLoopFunction();

	bool IsBreakpointHit() const;
	bool IsInstructionBreakpointHit() const;
	bool IsWatchpointHit(OpCode next_opcode) const;

	bool IsReadWatchpointHit(Memory::Address address) const;
	bool IsWriteWatchpointHit(Memory::Address address) const;

	// Listener notification
	void NotifyBreakpointHit(Memory::Address breakpoint) const;
	void NotifyInstructionBreakpointHit(OpCode opcode) const;
	void NotifyWatchpointHit(Memory::Watchpoint watchpoint) const;

private:
	BreakpointList breakpoints_;
	InstructionBreakpointList instruction_breakpoints_;

	// Watchpoints
	std::set<Memory::Address> read_watchpoints_;
	std::set<Memory::Address> write_watchpoints_;

	std::set<Listener*> listeners_;

private:
	DebugCPU(const DebugCPU&) = delete;
	DebugCPU(DebugCPU&&) = delete;
	DebugCPU& operator=(const DebugCPU&) = delete;
	DebugCPU& operator=(DebugCPU&&) = delete;
};
