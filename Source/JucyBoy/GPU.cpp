#include "GPU.h"
#include <string>

GPU::GPU(MMU &mmu) :
	mmu_(&mmu)
{
	//TODO: Trigger correct initial values in memory
	SetLineNumber(0);
	SetLcdState(State::OAM);
}

GPU::~GPU()
{

}

void GPU::OnCyclesLapsed(CPU::MachineCycles cycles)
{
	if (!lcd_on_) return;

	cycles_lapsed_in_state_ += cycles;
	switch (current_state_)
	{
	case State::OAM:
		if (cycles_lapsed_in_state_ >= 20)
		{
			cycles_lapsed_in_state_ -= 20;
			SetLcdState(State::VRAM);
		}
		break;
	case State::VRAM:
		if (cycles_lapsed_in_state_ >= 43)
		{
			cycles_lapsed_in_state_ -= 43;
			SetLcdState(State::HBLANK);

			RenderBackground(current_line_);
			// Render window
			// Render sprites
		}
		break;
	case State::HBLANK:
		if (cycles_lapsed_in_state_ >= 51)
		{
			cycles_lapsed_in_state_ -= 51;
			if (IncrementLine() == 144)
			{
				SetLcdState(State::VBLANK);

				for (int i = 0; i < color_numbers_buffer_.size(); ++i)
				{
					framebuffer_[i] = palette_[color_numbers_buffer_[i]];
				}

				NotifyNewFrame();

				// Request VBlank interrupt
				mmu_->SetBit<0>(Memory::interrupt_flags_register_);
			}
			else
			{
				SetLcdState(State::OAM);
			}
		}
		break;
	case State::VBLANK:
		if (cycles_lapsed_in_state_ >= 114)
		{
			cycles_lapsed_in_state_ -= 114;
			if (IncrementLine() == 0)
			{
				SetLcdState(State::OAM);
			}
			else
			{
				SetLcdState(State::VBLANK);
			}
		}
		break;
	default:
		throw std::logic_error("Invalid current mode in OnCyclesLapsed: " + std::to_string(cycles_lapsed_in_state_));
	}
}

void GPU::RenderBackground(uint8_t line_number)
{
	auto current_x = scroll_x_;
	auto current_y = line_number;
	current_y += scroll_y_;

	for (int i = 0; i < 160; ++i)
	{
		// Retrieve the tile number from the active tile map
		auto tile_number = tile_maps_[active_bg_tile_map_][32 * (current_y >> 3) + (current_x >> 3)];

		// Select tile depending on which tile set is currently active
		auto& tile = active_tile_set_ ? tile_set_[tile_number] : tile_set_[256 + static_cast<int8_t>(tile_number)];

		// The values in the tile have already been computed from successive bytes in VRAM during OnVramWritten, and can directly be used
		color_numbers_buffer_[160 * line_number + i] = tile[8 * (current_y & 0x07) + (current_x & 0x07)];

		++current_x;
	}
}

uint8_t GPU::SetLineNumber(uint8_t line_number)
{
	current_line_ = line_number;
	if (current_line_ >= 154)
	{
		current_line_ = 0;
	}

	mmu_->WriteByte(Memory::current_line_register_, current_line_, false);

	UpdateLineComparison();

	return current_line_;
}

void GPU::UpdateLineComparison()
{
	if (current_line_ == line_compare_)
	{
		mmu_->SetBit<2>(Memory::lcd_status_register_, false);

		// Request interrupt if enabled
		if (line_coincidence_interrupt_enabled_) { mmu_->SetBit<1>(Memory::interrupt_flags_register_); }
	}
	else
	{
		mmu_->ClearBit<2>(Memory::lcd_status_register_, false);
	}
}

void GPU::SetLcdControl(uint8_t value)
{
	show_bg_ = (value & (1 << 0)) != 0;
	show_sprites_ = (value & (1 << 1)) != 0;
	double_size_sprites_ = (value & (1 << 2)) != 0;
	active_bg_tile_map_ = (value & (1 << 3)) != 0 ? 1 : 0;
	active_tile_set_ = (value & (1 << 4)) != 0 ? 1 : 0;
	show_window_ = (value & (1 << 5)) != 0;
	active_window_tile_map_ = (value & (1 << 6)) != 0 ? 1 : 0;
	EnableLcd((value & (1 << 7)) != 0);
}

void GPU::SetLcdStatus(uint8_t value)
{
	hblank_interrupt_enabled_ = (value & (1 << 3)) != 0;
	vblank_interrupt_enabled_ = (value & (1 << 4)) != 0;
	oam_interrupt_enabled_ = (value & (1 << 5)) != 0;
	line_coincidence_interrupt_enabled_ = (value & (1 << 6)) != 0;
}

void GPU::SetPaletteData(uint8_t value)
{
	palette_[0] = static_cast<Color>(value & 0x03);
	palette_[1] = static_cast<Color>((value >> 2) & 0x03);
	palette_[2] = static_cast<Color>((value >> 4) & 0x03);
	palette_[3] = static_cast<Color>((value >> 6) & 0x03);
}

#pragma region Helper functions
void GPU::EnableLcd(bool enabled)
{
	if (lcd_on_ == enabled) return;

	//TODO: log message if disabled in states other than VBLANK (as it can damage the hardware)
	//TODO: change the framebuffer to a white screen

	lcd_on_ = enabled;

	if (lcd_on_)
	{
		//TODO: is it correct to set the beginning of the first line here?
		cycles_lapsed_in_state_ = 0;
		current_state_ = State::OAM;
		SetLineNumber(0);
	}
	else
	{
		//TODO: is it correct to set the end of the last line here? (while LCD is off, the stat register does indeed report VBlank state)
		cycles_lapsed_in_state_ = 114;
		current_state_ = State::VBLANK;
		SetLineNumber(153);
	}

	//TODO: does any interrupt have to be requested when turning LCD on/off? I guess not
}

void GPU::SetLcdState(State state)
{
	current_state_ = state;

	// Request interrupt if enabled
	switch (current_state_)
	{
	case State::HBLANK:
		if (hblank_interrupt_enabled_) { mmu_->SetBit<1>(Memory::interrupt_flags_register_); }
		break;
	case State::VBLANK:
		if (vblank_interrupt_enabled_) { mmu_->SetBit<1>(Memory::interrupt_flags_register_); }
		break;
	case State::OAM:
		if (oam_interrupt_enabled_) { mmu_->SetBit<1>(Memory::interrupt_flags_register_); }
		break;
	default:
		break;
	}
}
#pragma endregion

#pragma region MMU listener functions
void GPU::OnVramWritten(Memory::Address address, uint8_t value)
{
	if (address < Memory::tile_map_0_start_)
	{
		// Select tile from set, taking into account each tile is 16 bytes in size
		auto& tile = tile_set_[(address - Memory::tile_sets_start_) >> 4];

		// Pixel values in the tile are computed by combining the pertinent bit of two consecutive bytes in VRAM
		// Therefore updating one byte in VRAM will change the value of all 8 pixels in one line of the tile
		const auto line_in_tile = ((address - Memory::tile_sets_start_) & 0x0F) >> 1;
		for (int i = 0; i < 8; ++i)
		{
			const auto pixel_mask = (1 << (7 - i));
			const auto pixel_low_bit = static_cast<uint8_t>((mmu_->ReadByte(address & 0xFFFE) & pixel_mask) != 0 ? 1 : 0);
			const auto pixel_high_bit = static_cast<uint8_t>((mmu_->ReadByte((address & 0xFFFE) + 1) & pixel_mask) != 0 ? 1 : 0);
			tile[8 * line_in_tile + i] = pixel_low_bit + 2 * pixel_high_bit;
		}
	}
	else if (address < Memory::tile_map_1_start_)
	{
		tile_maps_[0][address - Memory::tile_map_0_start_] = value;
	}
	else
	{
		tile_maps_[1][address - Memory::tile_map_1_start_] = value;
	}
}

void GPU::OnOamWritten(Memory::Address /*address*/, uint8_t /*value*/)
{
	//TODO: sprite support
}

void GPU::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	switch (address)
	{
	case Memory::lcd_control_register_: // LCDC
		SetLcdControl(value);
		break;
	case Memory::lcd_status_register_: // STAT
		SetLcdStatus(value);
		break;
	case Memory::scroll_y_register_: // SCY
		scroll_y_ = value;
		break;
	case Memory::scroll_x_register_: // SCX
		scroll_x_ = value;
		break;
	case Memory::current_line_register_: // LY
		// Writing into this register resets the line counter
		SetLineNumber(0);

		//TODO: Should the cycle counter in the current mode be reset to 0 too?
		cycles_lapsed_in_state_ = 0;
		current_state_ = State::OAM;
		break;
	case Memory::line_compare_register_: // LYC
		line_compare_ = value;
		UpdateLineComparison();
		break;
	case Memory::dma_transfer_source_register_: // DMA
		//TODO: implement DMA transfer
		break;
	case Memory::bg_palette_register_: // BGP
		SetPaletteData(value);
		break;
	case Memory::obj_palette_0_register_: // OBP0
		//TODO: implement sprites
		break;
	case Memory::obj_palette_1_register_: // OBP1
		//TODO: implement sprites
		break;
	case Memory::window_y_register_: // WY
		window_y_ = value;
		break;
	case Memory::window_x_minus_seven_register_: // WX
		window_x_ = value;
		break;
	}
}
#pragma endregion

#pragma region Listener notification
void GPU::NotifyNewFrame() const
{
	for (auto& listener : listeners_)
	{
		listener->OnNewFrame(framebuffer_);
	}
}
#pragma endregion
