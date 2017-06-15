#pragma once

#include "Memory.h"
#include <atomic>
#include <vector>

class Joypad final
{
public:
	enum class Keys
	{
		Down,	// bit 3
		Up,		// bit 2
		Left,	// bit 1
		Right,	// bit 0
		Start,	// bit 3
		Select,	// bit 2
		B,		// bit 1
		A		// bit 0
	};

	Joypad() = default;
	~Joypad() = default;

	// MMU mapped memory read/write functions
	uint8_t OnIoMemoryRead(Memory::Address address) const;
	void OnIoMemoryWritten(Memory::Address address, uint8_t value);

	void UpdatePressedKeys(std::vector<Keys> pressed_keys);

private:
	std::atomic<uint8_t> pressed_buttons_{ 0xFF };
	std::atomic<uint8_t> pressed_directions_{ 0xFF };

	bool direction_keys_requested_{ true };
	bool button_keys_requested_{ true };
};
