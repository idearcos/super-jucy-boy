#pragma once

#include "../CPU.h"

class MMU;

class DebugCPU final : public CPU
{
public:
	using BreakpointList = std::set<uint16_t>;
	using InstructionBreakpointList = std::set<OpCode>;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnBreakpointHit(uint16_t /*breakpoint*/) {}
		virtual void OnInstructionBreakpointHit(OpCode &/*opcode*/) {}
	};

	DebugCPU(MMU &mmu);
	~DebugCPU() = default;

	void DebugRun();
	void DebugStepOver();

	// Status retrieval
	inline Registers GetRegistersState() const { return registers_; }
	inline Flags GetFlagsState() const { return ReadFlags(); }
	inline BreakpointList GetBreakpoints() const { return breakpoints_; }
	inline InstructionBreakpointList GetInstructionBreakpoints() const { return instruction_breakpoints_; }

	// Breakpoints
	inline void AddBreakpoint(Memory::Address address) { breakpoints_.insert(address); } //TODO: allow only when not running!
	inline void RemoveBreakpoint(Memory::Address address) { breakpoints_.erase(address); } //TODO: allow only when not running!
	inline void AddInstructionBreakpoint(OpCode opcode) { instruction_breakpoints_.insert(opcode); } //TODO: allow only when not running!
	inline void RemoveInstructionBreakpoint(OpCode opcode) { instruction_breakpoints_.erase(opcode); } //TODO: allow only when not running!

	// Watchpoints
	std::vector<Memory::Watchpoint> GetWatchpointList() const;
	void AddWatchpoint(Memory::Watchpoint watchpoint);
	void RemoveWatchpoint(Memory::Watchpoint watchpoint);
	
	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	void DebugRunningLoopFunction();

	inline bool IsBreakpointHit() const { return (breakpoints_.find(registers_.pc) != breakpoints_.end()); }
	bool IsInstructionBreakpointHit() const;
	bool IsWatchpointHit(OpCode next_opcode) const;

	inline bool IsReadWatchpointHit(Memory::Address address) const { return read_watchpoints_.count(address) != 0; }
	inline bool IsWriteWatchpointHit(Memory::Address address) const { return write_watchpoints_.count(address) != 0; }

	// Listener notification

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
