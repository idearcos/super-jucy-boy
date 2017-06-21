#include "PPU.h"
#include <string>
#include <cassert>
#include "MMU.h"

PPU::PPU(MMU &mmu) :
	mmu_(&mmu),
	bg_palette_{ Color::Black, Color::Black, Color::Black, Color::White },
	obj_palettes_{ { { Color::Black, Color::Black, Color::Black, Color::Black },
		{ Color::Black, Color::Black, Color::Black, Color::Black } } }
{

}

void PPU::OnMachineCycleLapse()
{
	if (lcd_on_)
	{
		current_state_ = next_state_;
		clock_cycles_lapsed_in_state_ += 4;
		switch (current_state_)
		{
		case State::EnteredOAM:
			if (oam_interrupt_enabled_) { mmu_->SetBit(Memory::IF, 1); }
			next_state_ = State::OAM;
		case State::OAM:
			if (clock_cycles_lapsed_in_state_ >= oam_state_duration_)
			{
				clock_cycles_lapsed_in_state_ -= oam_state_duration_;
				vram_duration_this_line_ = vram_state_duration_ + (scroll_x_ & 0x07);
				next_state_ = State::VRAM;
			}
			break;
		case State::VRAM:
			if (clock_cycles_lapsed_in_state_ >= vram_duration_this_line_)
			{
				clock_cycles_lapsed_in_state_ -= vram_duration_this_line_;
				hblank_duration_this_line_ = line_duration_ - oam_state_duration_ - vram_duration_this_line_;
				next_state_ = State::EnteredHBLANK;

				RenderBackground(current_line_);
				RenderWindow(current_line_);
				RenderSprites(current_line_);
			}
			break;
		case State::EnteredHBLANK:
			if (hblank_interrupt_enabled_) { mmu_->SetBit(Memory::IF, 1); }
			next_state_ = State::HBLANK;
		case State::HBLANK:
			if (clock_cycles_lapsed_in_state_ >= hblank_duration_this_line_)
			{
				clock_cycles_lapsed_in_state_ -= hblank_duration_this_line_;
				if (IncrementLine() == 144)
				{
					next_state_ = State::EnteredVBLANK;

					NotifyNewFrame();

					// Request VBlank interrupt
					mmu_->SetBit(Memory::IF, 0);
				}
				else
				{
					next_state_ = State::EnteredOAM;
				}
			}
			break;
		case State::EnteredVBLANK:
			if (vblank_interrupt_enabled_) { mmu_->SetBit(Memory::IF, 1); }
			next_state_ = State::VBLANK;
		case State::VBLANK:
			if (clock_cycles_lapsed_in_state_ >= line_duration_)
			{
				clock_cycles_lapsed_in_state_ -= line_duration_;
				if (IncrementLine() == 0)
				{
					next_state_ = State::EnteredOAM;
				}
				else
				{
					next_state_ = State::VBLANK;
				}
			}
			break;
		default:
			throw std::logic_error("Invalid current mode in OnMachineCycleLapse: " + std::to_string(static_cast<int>(current_state_)));
		}
	}

	// OAM DMA
	oam_dma_.current_state_ = oam_dma_.next_state_;
	switch (oam_dma_.current_state_)
	{
	case OamDma::State::Startup:
		oam_dma_.current_byte_index_ = 0;
		oam_dma_.next_state_ = OamDma::State::Active;
		break;
	case OamDma::State::Active:
		WriteOam(oam_dma_.current_byte_index_, mmu_->ReadByte(oam_dma_.source_ + oam_dma_.current_byte_index_));

		if (++oam_dma_.current_byte_index_ == Memory::oam_size_)
		{
			oam_dma_.current_byte_index_ = 0;
			oam_dma_.next_state_ = OamDma::State::Teardown;
			break;
		}

		break;
	case OamDma::State::Teardown:
		oam_dma_.next_state_ = OamDma::State::Inactive;
		break;
	case OamDma::State::Inactive:
		break;
	default:
		throw std::logic_error{ "Invalid OAM DMA state: " + std::to_string(static_cast<size_t>(oam_dma_.current_state_)) };
	}
}

void PPU::RenderBackground(uint8_t line_number)
{
	if (!show_bg_) return;

	auto scrolled_x = scroll_x_;
	const auto scrolled_y = static_cast<uint8_t>(line_number + scroll_y_);

	uint8_t color_number{ 0 };
	for (int i = 0; i < 160; ++i)
	{
		// Retrieve the tile number from the active tile map
		auto tile_number = tile_maps_[active_bg_tile_map_][32 * (scrolled_y >> 3) + (scrolled_x >> 3)];

		// Select tile depending on which tile set is currently active
		auto& tile = active_tile_set_ ? tile_set_[tile_number] : tile_set_[256 + static_cast<int8_t>(tile_number)];

		// The values in the tile have already been computed from successive bytes in VRAM during OnVramWritten, and can directly be used
		color_number = tile[8 * (scrolled_y & 0x07) + (scrolled_x & 0x07)];

		is_bg_transparent_[160 * line_number + i] = (color_number == 0);

		framebuffer_[160 * line_number + i] = bg_palette_[color_number];

		scrolled_x += 1;
	}
}

void PPU::RenderWindow(uint8_t line_number)
{
	if (!show_window_) return;

	if (line_number < window_y_) return;

	const auto window_line = static_cast<uint8_t>(line_number - window_y_);
	const auto end_of_window = (window_x_ + 160);

	uint8_t color_number{ 0 };
	for (int x = window_x_; x < end_of_window; ++x)
	{
		if (x < 0) continue;
		if (x >= 160) break;

		const auto window_x = x - window_x_;

		// Retrieve the tile number from the active tile map
		auto tile_number = tile_maps_[active_window_tile_map_][32 * (window_line >> 3) + (window_x >> 3)];

		// Select tile depending on which tile set is currently active
		auto& tile = active_tile_set_ ? tile_set_[tile_number] : tile_set_[256 + static_cast<int8_t>(tile_number)];

		// The values in the tile have already been computed from successive bytes in VRAM during OnVramWritten, and can directly be used
		color_number = tile[8 * (window_line & 0x07) + (window_x & 0x07)];

		is_bg_transparent_[160 * line_number + x] = (color_number == 0);

		framebuffer_[160 * line_number + x] = bg_palette_[color_number];
	}
}

void PPU::RenderSprites(uint8_t line_number)
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
		auto tile_line = line_number - sprite.GetY();
		if (sprite.IsVerticallyFlipped()) tile_line = (sprite_height - 1) - tile_line;

		// 16 pixel height sprites' first tile number is retrieved by resetting the lowest bit; the second tile number is retrieved by setting it.
		const auto& tile = (!double_size_sprites_) ? tile_set_[sprite.GetTileNumber()]
			: (tile_line < 8 ? tile_set_[sprite.GetTileNumber() & 0xFE] : tile_set_[sprite.GetTileNumber() | 0x01]);
		tile_line &= 0x07;

		uint8_t color_number{ 0 };
		for (auto x = sprite.GetX(), tile_x_offset = !sprite.IsHorizontallyFlipped() ? 0 : 7; x < sprite.GetX() + 8; ++x, !sprite.IsHorizontallyFlipped() ? ++tile_x_offset : --tile_x_offset)
		{
			if (x < 0 || x >= 160) continue;

			if (tile[8 * tile_line + tile_x_offset] == 0) continue;

			if (!sprite.IsRenderedAboveBackground() && !is_bg_transparent_[160 * line_number + x]) continue;

			color_number = tile[8 * tile_line + tile_x_offset];
			framebuffer_[160 * line_number + x] = obj_palettes_[sprite.GetObjPaletteNumber()][color_number];
		}
	}
}

void PPU::SetLcdControl(uint8_t value)
{
	show_bg_ = (value & 0x01) != 0;
	show_sprites_ = (value & 0x02) != 0;
	double_size_sprites_ = (value & 0x04) != 0;
	active_bg_tile_map_ = (value & 0x08) != 0 ? 1 : 0;
	active_tile_set_ = (value & 0x10) != 0 ? 1 : 0;
	show_window_ = (value & 0x20) != 0;
	active_window_tile_map_ = (value & 0x40) != 0 ? 1 : 0;
	EnableLcd((value & 0x80) != 0);
}

void PPU::SetLcdStatus(uint8_t value)
{
	hblank_interrupt_enabled_ = (value & 0x08) != 0;
	vblank_interrupt_enabled_ = (value & 0x10) != 0;
	oam_interrupt_enabled_ = (value & 0x20) != 0;
	line_coincidence_interrupt_enabled_ = (value & 0x40) != 0;
}

void PPU::SetPaletteData(Palette &palette, uint8_t value)
{
	palette[0] = static_cast<Color>(value & 0x03);
	palette[1] = static_cast<Color>((value >> 2) & 0x03);
	palette[2] = static_cast<Color>((value >> 4) & 0x03);
	palette[3] = static_cast<Color>((value >> 6) & 0x03);
}

#pragma region Helper functions
void PPU::EnableLcd(bool enabled)
{
	if (lcd_on_ == enabled) return;

	//TODO: log message if disabled in states other than VBLANK (as it can damage the hardware)
	//TODO: change the framebuffer to a white screen

	lcd_on_ = enabled;

	if (lcd_on_)
	{
		clock_cycles_lapsed_in_state_ = 4;
		next_state_ = State::OAM;
		SetLineNumber(0);
	}
	else
	{
		current_state_ = State::VBLANK;
		SetLineNumber(153);
	}
}

uint8_t PPU::SetLineNumber(uint8_t line_number)
{
	current_line_ = line_number;
	if (current_line_ >= 154)
	{
		current_line_ = 0;
	}

	UpdateLineComparison();

	return current_line_;
}

void PPU::UpdateLineComparison()
{
	// Request interrupt if enabled
	if ((current_line_ == line_compare_) && line_coincidence_interrupt_enabled_) { mmu_->SetBit(Memory::IF, 1); }
}

uint8_t PPU::GetPaletteData(const Palette &palette) const
{
	return (static_cast<uint8_t>(palette[0]))
		| (static_cast<uint8_t>(palette[1]) << 2)
		| (static_cast<uint8_t>(palette[2]) << 4)
		| (static_cast<uint8_t>(palette[3]) << 6);
}
#pragma endregion

#pragma region MMU mapped memory read/write functions
uint8_t PPU::OnVramRead(Memory::Address relative_address) const
{
	if (current_state_ == State::VRAM) return 0xFF;

	return vram_[relative_address];
}

void PPU::OnVramWritten(Memory::Address relative_address, uint8_t value)
{
	if (current_state_ == State::VRAM) return;

	vram_[relative_address] = value;

	if (relative_address < tile_map_0_offset_)
	{
		// Select tile from set, taking into account each tile is 16 bytes in size
		auto& tile = tile_set_[relative_address >> 4];

		// Pixel values in the tile are computed by combining the pertinent bit of two consecutive bytes in VRAM
		// Therefore updating one byte in VRAM will change the value of all 8 pixels in one line of the tile
		const auto line_in_tile = (relative_address & 0x0F) >> 1;
		for (int i = 0; i < 8; ++i)
		{
			const auto pixel_mask = (0x80 >> i);
			const auto pixel_low_bit = (vram_[relative_address & 0xFFFE] & pixel_mask) != 0 ? 1 : 0;
			const auto pixel_high_bit = (vram_[(relative_address & 0xFFFE) + 1] & pixel_mask) != 0 ? 1 : 0;
			tile[8 * line_in_tile + i] = static_cast<uint8_t>(pixel_low_bit + (pixel_high_bit << 1));
		}
	}
	else if (relative_address < tile_map_1_offset_)
	{
		tile_maps_[0][relative_address - tile_map_0_offset_] = value;
	}
	else
	{
		tile_maps_[1][relative_address - tile_map_1_offset_] = value;
	}
}

uint8_t PPU::OnOamRead(Memory::Address relative_address) const
{
	if ((current_state_ == State::VRAM) || (current_state_ == State::OAM) || (current_state_ == State::EnteredOAM)) return 0xFF;

	if (oam_dma_.current_state_ == OamDma::State::Active) return 0xFF;

	return oam_[relative_address];
}

void PPU::OnOamWritten(Memory::Address relative_address, uint8_t value)
{
	if ((current_state_ == State::VRAM) || (current_state_ == State::OAM) || (current_state_ == State::EnteredOAM)) return;

	if (oam_dma_.current_state_ == OamDma::State::Active) return;

	WriteOam(relative_address, value);
}

uint8_t PPU::OnIoMemoryRead(Memory::Address address)
{
	switch (address)
	{
	case Memory::LCDC:
	{uint8_t value{ 0 };
	value |= static_cast<uint8_t>(show_bg_);
	value |= static_cast<uint8_t>(show_sprites_) << 1;
	value |= static_cast<uint8_t>(double_size_sprites_) << 2;
	value |= static_cast<uint8_t>(active_bg_tile_map_) << 3;
	value |= static_cast<uint8_t>(active_tile_set_) << 4;
	value |= static_cast<uint8_t>(show_window_) << 5;
	value |= static_cast<uint8_t>(active_window_tile_map_) << 6;
	value |= static_cast<uint8_t>(lcd_on_) << 7;
	return value; }
	case Memory::STAT:
	{uint8_t value{ 0x80 };
	value |= static_cast<uint8_t>(current_state_) & 0x03;
	value |= static_cast<uint8_t>(current_line_ == line_compare_) << 2;
	value |= static_cast<uint8_t>(hblank_interrupt_enabled_) << 3;
	value |= static_cast<uint8_t>(vblank_interrupt_enabled_) << 4;
	value |= static_cast<uint8_t>(oam_interrupt_enabled_) << 5;
	value |= static_cast<uint8_t>(line_coincidence_interrupt_enabled_) << 6;
	return value; }
	case Memory::SCY:
		return scroll_y_;
	case Memory::SCX:
		return scroll_x_;
	case Memory::LY:
		return current_line_;
	case Memory::LYC:
		return line_compare_;
	case Memory::DMA:
		return static_cast<uint8_t>(oam_dma_.source_ >> 8);
	case Memory::BGP:
		return GetPaletteData(bg_palette_);
		break;
	case Memory::OBP0:
		return GetPaletteData(obj_palettes_[0]);
		break;
	case Memory::OBP1:
		return GetPaletteData(obj_palettes_[1]);
		break;
	case Memory::WY:
		return static_cast<uint8_t>(window_y_);
	case Memory::WX:
		return static_cast<uint8_t>(window_x_ + 7);
	default:
		throw std::invalid_argument{ "Reading from invalid memory address in PPU: " + address };
	}
}

void PPU::OnIoMemoryWritten(Memory::Address address, uint8_t value)
{
	switch (address)
	{
	case Memory::LCDC:
		SetLcdControl(value);
		break;
	case Memory::STAT:
		SetLcdStatus(value);
		break;
	case Memory::SCY:
		scroll_y_ = value;
		break;
	case Memory::SCX:
		scroll_x_ = value;
		break;
	case Memory::LY:
		// Writing into this register resets the line counter
		SetLineNumber(0);

		//TODO: Should the cycle counter in the current mode be reset to 0 too?
		clock_cycles_lapsed_in_state_ = 0;
		next_state_ = State::EnteredOAM;
		break;
	case Memory::LYC:
		line_compare_ = value;
		UpdateLineComparison();
		break;
	case Memory::DMA:
		if (value > 0xF1) throw std::invalid_argument("Invalid DMA transfer source: " + std::to_string(int{ value }));

		oam_dma_.next_state_ = OamDma::State::Startup;
		oam_dma_.source_ = value << 8;
		break;
	case Memory::BGP:
		SetPaletteData(bg_palette_, value);
		break;
	case Memory::OBP0:
		SetPaletteData(obj_palettes_[0], value);
		break;
	case Memory::OBP1:
		SetPaletteData(obj_palettes_[1], value);
		break;
	case Memory::WY:
		window_y_ = value;
		break;
	case Memory::WX:
		window_x_ = value - 7;
		break;
	default:
		throw std::invalid_argument{ "Writing to invalid memory address in PPU: " + address };
	}
}

void PPU::WriteOam(Memory::Address relative_address, uint8_t value)
{
	oam_[relative_address] = value;

	// There are 40 sprites, 4 bytes each, held in OAM [0xFE00 - 0xFE9F]
	const auto sprite_num = relative_address >> 2;
	assert((sprite_num >= 0) && (sprite_num < 40));

	switch (relative_address & 0x03)
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
#pragma endregion

#pragma region Listener notification
void PPU::NotifyNewFrame() const
{
	for (auto& listener : listeners_)
	{
		listener->OnNewFrame(framebuffer_);
	}
}
#pragma endregion
