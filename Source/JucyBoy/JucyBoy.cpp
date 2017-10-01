#include "JucyBoy.h"

JucyBoy::JucyBoy(const std::string &rom_file_path) :
	cartridge_{ rom_file_path }
{
	// Register all listener functions
	// CPU listeners
	cpu_.CPU::AddListener(timer_);
	cpu_.CPU::AddListener(ppu_);
	cpu_.CPU::AddListener(apu_);

	// Map memory read/write functions to MMU
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return ppu_.OnVramRead(relative_address); }, Memory::Region::VRAM);
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return ppu_.OnOamRead(relative_address); }, Memory::Region::OAM);
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return cpu_.OnInterruptsRead(relative_address); }, Memory::Region::Interrupts);

	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { ppu_.OnVramWritten(relative_address, value); }, Memory::Region::VRAM);
	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { ppu_.OnOamWritten(relative_address, value); }, Memory::Region::OAM);
	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cpu_.OnInterruptsWritten(relative_address, value); }, Memory::Region::Interrupts);

	// Map IO register read/write functions to MMU
	mmu_.MapIoRegisterRead([this](Memory::Address relative_address) { return cpu_.OnIoMemoryRead(relative_address); }, Memory::IF, Memory::IF);
	mmu_.MapIoRegisterRead([this](Memory::Address relative_address) { return ppu_.OnIoMemoryRead(relative_address); }, Memory::LCDC, Memory::WX);
	mmu_.MapIoRegisterRead([this](Memory::Address relative_address) { return apu_.OnIoMemoryRead(relative_address); }, Memory::NR10, Memory::WaveEnd);
	mmu_.MapIoRegisterRead([this](Memory::Address relative_address) { return timer_.OnIoMemoryRead(relative_address); }, Memory::DIV, Memory::TAC);
	mmu_.MapIoRegisterRead([this](Memory::Address relative_address) { return joypad_.OnIoMemoryRead(relative_address); }, Memory::JOYP, Memory::JOYP);
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return cartridge_.OnRomBank0Read(relative_address); }, Memory::Region::ROM_Bank0);
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return cartridge_.OnRomBankNRead(relative_address); }, Memory::Region::ROM_OtherBanks);
	mmu_.MapMemoryRead([this](Memory::Address relative_address) { return cartridge_.OnExternalRamRead(relative_address); }, Memory::Region::ERAM);

	mmu_.MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { cpu_.OnIoMemoryWritten(relative_address, value); }, Memory::IF, Memory::IF);
	mmu_.MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { ppu_.OnIoMemoryWritten(relative_address, value); }, Memory::LCDC, Memory::WX);
	mmu_.MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { apu_.OnIoMemoryWritten(relative_address, value); }, Memory::NR10, Memory::WaveEnd);
	mmu_.MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { timer_.OnIoMemoryWritten(relative_address, value); }, Memory::DIV, Memory::TAC);
	mmu_.MapIoRegisterWrite([this](Memory::Address relative_address, uint8_t value) { joypad_.OnIoMemoryWritten(relative_address, value); }, Memory::JOYP, Memory::JOYP);
	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_.OnRomBank0Written(relative_address, value); }, Memory::Region::ROM_Bank0);
	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_.OnRomBankNWritten(relative_address, value); }, Memory::Region::ROM_OtherBanks);
	mmu_.MapMemoryWrite([this](Memory::Address relative_address, uint8_t value) { cartridge_.OnExternalRamWritten(relative_address, value); }, Memory::Region::ERAM);
}

void JucyBoy::StartEmulation(bool debug)
{
	debug ? cpu_.DebugRun() : cpu_.Run();
}

void JucyBoy::PauseEmulation()
{
	cpu_.Stop();
}

void JucyBoy::StepOver(bool debug)
{
	debug ? cpu_.DebugStepOver() : cpu_.StepOver();
}
