#include "Joypad.h"
#include "MMU.h"

Joypad::Joypad(MMU &mmu) :
	mmu_{ &mmu }
{

}

void Joypad::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	if (address != Memory::JOYP) return;
	
	const auto direction_keys_requested = (value & (1 << 4)) == 0;
	const auto button_keys_requested = (value & (1 << 5)) == 0;

	uint8_t pressed_keys{ 0xFF };
	if (direction_keys_requested) pressed_keys &= pressed_directions_.load();
	if (button_keys_requested) pressed_keys &= pressed_buttons_.load();

	mmu_->WriteByte(Memory::JOYP, pressed_keys, false);
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
			pressed_directions &= ~(1 << 3);
			break;
		case Joypad::Keys::Up:
			pressed_directions &= ~(1 << 2);
			break;
		case Joypad::Keys::Left:
			pressed_directions &= ~(1 << 1);
			break;
		case Joypad::Keys::Right:
			pressed_directions &= ~(1 << 0);
			break;
		case Joypad::Keys::Start:
			pressed_buttons &= ~(1 << 3);
			break;
		case Joypad::Keys::Select:
			pressed_buttons &= ~(1 << 2);
			break;
		case Joypad::Keys::B:
			pressed_buttons &= ~(1 << 1);
			break;
		case Joypad::Keys::A:
			pressed_buttons &= ~(1 << 0);
			break;
		default:
			break;
		}
	}

	pressed_directions_.store(pressed_directions);
	pressed_buttons_.store(pressed_buttons);
}
