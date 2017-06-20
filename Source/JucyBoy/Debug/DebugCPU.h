#pragma once

#include "../CPU.h"

class DebugMMU;

class DebugCPU final : public CPU
{
public:
	using BreakpointList = std::set<uint16_t>;
	using InstructionBreakpointList = std::set<OpCode>;

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void OnBreakpointsChanged(const BreakpointList &/*breakpoint_list*/) {}
		virtual void OnInstructionBreakpointsChanged(const InstructionBreakpointList &/*instruction_breakpoint_list*/) {}
	};

	DebugCPU(DebugMMU &mmu);
	~DebugCPU() = default;

	void Run() override;
	void StepOver() override;

	// Status retrieval
	inline Registers GetRegistersState() const { return registers_; }
	inline Flags GetFlagsState() const { return ReadFlags(); }

	// Breakpoints
	void AddBreakpoint(Memory::Address address);
	void RemoveBreakpoint(Memory::Address address);
	void AddInstructionBreakpoint(OpCode opcode);
	void RemoveInstructionBreakpoint(OpCode opcode);
	
	// Listeners management
	void AddListener(Listener &listener) { listeners_.insert(&listener); }
	void RemoveListener(Listener &listener) { listeners_.erase(&listener); }

private:
	void DebugRunningLoopFunction();

	inline bool IsBreakpointHit() const { return (breakpoints_.find(registers_.pc) != breakpoints_.end()); }
	bool IsInstructionBreakpointHit() const;
	bool IsWatchpointHit(OpCode next_opcode) const;

	// Listener notification
	void NotifyBreakpointsChange() const;
	void NotifyInstructionBreakpointsChange() const;

private:
	BreakpointList breakpoints_;
	InstructionBreakpointList instruction_breakpoints_;

	std::set<Listener*> listeners_;

	DebugMMU *debug_mmu_{ nullptr };

private:
	DebugCPU(const DebugCPU&) = delete;
	DebugCPU(DebugCPU&&) = delete;
	DebugCPU& operator=(const DebugCPU&) = delete;
	DebugCPU& operator=(DebugCPU&&) = delete;
};
