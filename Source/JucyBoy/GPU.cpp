#include "GPU.h"
#include <string>
#include <cassert>
#include <algorithm>
#include "MMU.h"

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
			RenderWindow(current_line_);
			RenderSprites(current_line_);
		}
		break;
	case State::HBLANK:
		if (cycles_lapsed_in_state_ >= 51)
		{
			cycles_lapsed_in_state_ -= 51;
			if (IncrementLine() == 144)
			{
				SetLcdState(State::VBLANK);

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
	if (!show_bg_) return;

	auto scrolled_x = scroll_x_;
	const auto scrolled_y = static_cast<uint8_t>(line_number + scroll_y_);

	for (int i = 0; i < 160; ++i)
	{
		// Retrieve the tile number from the active tile map
		auto tile_number = tile_maps_[active_bg_tile_map_][32 * (scrolled_y >> 3) + (scrolled_x >> 3)];

		// Select tile depending on which tile set is currently active
		auto& tile = active_tile_set_ ? tile_set_[tile_number] : tile_set_[256 + static_cast<int8_t>(tile_number)];

		// The values in the tile have already been computed from successive bytes in VRAM during OnVramWritten, and can directly be used
		color_numbers_buffer_[160 * line_number + i] = tile[8 * (scrolled_y & 0x07) + (scrolled_x & 0x07)];

		framebuffer_[160 * line_number + i] = bg_palette_[color_numbers_buffer_[160 * line_number + i]];

		++scrolled_x;
	}
}

void GPU::RenderWindow(uint8_t line_number)
{
	if (!show_window_) return;

	if (line_number < window_y_) return;

	const auto window_line = static_cast<uint8_t>(line_number - window_y_);

	for (int x = window_x_; x < 160; ++x)
	{
		if ((x < 0) || (x >= 160)) continue;

		// Retrieve the tile number from the active tile map
		auto tile_number = tile_maps_[active_window_tile_map_][32 * (window_line >> 3) + (x >> 3)];

		// Select tile depending on which tile set is currently active
		auto& tile = active_tile_set_ ? tile_set_[tile_number] : tile_set_[256 + static_cast<int8_t>(tile_number)];

		// The values in the tile have already been computed from successive bytes in VRAM during OnVramWritten, and can directly be used
		color_numbers_buffer_[160 * line_number + x] = tile[8 * (window_line & 0x07) + (x & 0x07)];

		framebuffer_[160 * line_number + x] = bg_palette_[color_numbers_buffer_[160 * line_number + x]];
	}
}

void GPU::RenderSprites(uint8_t line_number)
{
	if (!show_sprites_) return;

	// Only 10 sprites can be displayed on any one line, prioritized by address (i.e. 0xFE00 highest, 0xFE04 next highest, etc.)
	// When this limit is exceeded, the lower priority sprites won't be displayed
	// Among the sprites to be displayed, the drawing priority is defined as follows:
	//   Sprites with lower x coordinate (closer to the left) have higher priority and appear above any others
	//   When sprites with the same x coordinate values overlap, they have priority according to table ordering (i.e. 0xFE00 highest, 0xFE04 next highest, etc.)

	//TODO: precompute sprites priority when OAM is written? That may perform better than rechecking priority every time

	std::vector<size_t> indices_of_sprites_to_render;

	// First, filter the sprites to be rendered in the current scanline
	const auto sprite_height = double_size_sprites_ ? 16 : 8;
	for (int i = 0; i < sprites_.size(); ++i)
	{
		// Verify if it has to be rendered in the current scanline
		if ((line_number < sprites_[i].GetY()) || (line_number >= (sprites_[i].GetY() + sprite_height))) { continue; }

		indices_of_sprites_to_render.emplace_back(i);

		if (indices_of_sprites_to_render.size() == 10) break;
	}

	// Remove the sprites that are off-screen, since they don't need to be rendered
	indices_of_sprites_to_render.erase(std::remove_if(indices_of_sprites_to_render.begin(), indices_of_sprites_to_render.end(), [this](size_t index) {
		return (sprites_[index].GetX() <= -8) || (sprites_[index].GetX() >= 160);
	}), indices_of_sprites_to_render.end());

	// Return if there are no sprites to render
	if (indices_of_sprites_to_render.empty()) return;

	// Now sort the remaining sprites according to rendering priority: draw higher X (or higher address, if equals X) sprites first (i.e. from right to left)
	// This way, in the final image the sprites with lower X and lower address will appear above the rest
	std::sort(indices_of_sprites_to_render.begin(), indices_of_sprites_to_render.end(), [this](size_t lhs_index, size_t rhs_index) {
		return sprites_[lhs_index].GetX() > sprites_[rhs_index].GetX()
			|| (sprites_[lhs_index].GetX() == sprites_[rhs_index].GetX()) && (lhs_index > rhs_index);
	});

	// Render the sprites one by one, right to left
	for (int i = 0; i < indices_of_sprites_to_render.size(); ++i)
	{
		const auto& sprite = sprites_[indices_of_sprites_to_render[i]];
		const auto& tile = tile_set_[sprite.GetTileNumber()];
		const auto tile_line = line_number - sprite.GetY();

		for (auto x = sprite.GetX(), tile_x_offset = 0; x < sprite.GetX() + 8; ++x, ++tile_x_offset)
		{
			if (x < 0 || x >= 160) continue;

			if (tile[8 * tile_line + tile_x_offset] == 0) continue;

			if (!sprite.IsRenderedAboveBackground() && (color_numbers_buffer_[160 * line_number + x] != 0)) continue;

			color_numbers_buffer_[160 * line_number + x] = tile[8 * tile_line + tile_x_offset];

			framebuffer_[160 * line_number + x] = obj_palettes_[sprite.GetObjPaletteNumber()][color_numbers_buffer_[160 * line_number + x]];
		}
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

void GPU::SetPaletteData(Palette &palette, uint8_t value)
{
	palette[0] = static_cast<Color>(value & 0x03);
	palette[1] = static_cast<Color>((value >> 2) & 0x03);
	palette[2] = static_cast<Color>((value >> 4) & 0x03);
	palette[3] = static_cast<Color>((value >> 6) & 0x03);
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

void GPU::OnOamWritten(Memory::Address address, uint8_t value)
{
	// There are 40 sprites, 4 bytes each, held in OAM [0xFE00 - 0xFE9F]
	const auto sprite_num = (address - Memory::oam_start_) >> 2;
	assert((sprite_num >= 0) && (sprite_num < 40));

	switch (address & 0x03)
	{
	case 0:
		sprites_[sprite_num].SetY(value);
		break;
	case 1:
		sprites_[sprite_num].SetX(value);
		break;
	case 2:
		sprites_[sprite_num].SetTileNumber(value);
		break;
	case 3:
		sprites_[sprite_num].SetOptions(value);
		break;
	}
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
		if (value > 0xF1) throw std::invalid_argument("Invalid DMA transfer source: " + std::to_string(int{ value }));
		for (uint16_t i = 0; i < 160; ++i)
		{
			mmu_->WriteByte(Memory::oam_start_ + i, mmu_->ReadByte((value << 8) + i));
		}
		break;
	case Memory::bg_palette_register_: // BGP
		SetPaletteData(bg_palette_, value);
		break;
	case Memory::obj_palette_0_register_: // OBP0
		SetPaletteData(obj_palettes_[0], value);
		break;
	case Memory::obj_palette_1_register_: // OBP1
		SetPaletteData(obj_palettes_[1], value);
		break;
	case Memory::window_y_register_: // WY
		window_y_ = value;
		break;
	case Memory::window_x_minus_seven_register_: // WX
		window_x_ = value - 7;
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
