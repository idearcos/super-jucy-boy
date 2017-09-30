#include "Joypad.h"
#include <cassert>

uint8_t Joypad::OnIoMemoryRead(Memory::Address /*address*/) const
{
	uint8_t pressed_keys{ 0xFF };
	if (direction_keys_requested_)
	{
		pressed_keys &= ~0x10;
		pressed_keys &= pressed_directions_.load();
	}
	if (button_keys_requested_)
	{
		pressed_keys &= ~0x20;
		pressed_keys &= pressed_buttons_.load();
	}

	return pressed_keys;
}

void Joypad::OnIoMemoryWritten(Memory::Address /*address*/, uint8_t value)
{
	direction_keys_requested_ = (value & 0x10) == 0;
	button_keys_requested_ = (value & 0x20) == 0;
}

void Joypad::UpdatePressedKeys(std::vector<Keys> pressed_keys)
{
	uint8_t pressed_directions{ 0xFF };
	uint8_t pressed_buttons{ 0xFF };

	for (auto key : pressed_keys)
	{
		switch (key)
		{
		case Joypad::Keys::Down:
			pressed_directions &= ~0x08;
			break;
		case Joypad::Keys::Up:
			pressed_directions &= ~0x04;
			break;
		case Joypad::Keys::Left:
			pressed_directions &= ~0x02;
			break;
		case Joypad::Keys::Right:
			pressed_directions &= ~0x01;
			break;
		case Joypad::Keys::Start:
			pressed_buttons &= ~0x08;
			break;
		case Joypad::Keys::Select:
			pressed_buttons &= ~0x04;
			break;
		case Joypad::Keys::B:
			pressed_buttons &= ~0x02;
			break;
		case Joypad::Keys::A:
			pressed_buttons &= ~0x01;
			break;
		default:
			break;
		}
	}

	pressed_directions_.store(pressed_directions);
	pressed_buttons_.store(pressed_buttons);
}
