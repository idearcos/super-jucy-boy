#pragma once

#include <cstdint>

class Sprite final
{
public:
	Sprite() = default;
	~Sprite() = default;

	void SetY(uint8_t y) { y_ = y - 16; }
	void SetX(uint8_t x) { x_ = x - 8; }
	void SetTileNumber(uint8_t tile_number) { tile_number_ = tile_number; }
	void SetOptions(uint8_t options)
	{
		obj_palette_number_ = ((options & (1 << 4)) != 0) ? 1 : 0;
		horizontal_flip_ = (options & (1 << 5)) != 0;
		vertical_flip_ = (options & (1 << 6)) != 0;
		render_above_background_ = (options & (1 << 7)) == 0;
	}

	int GetY() const { return y_; }
	int GetX() const { return x_; }
	uint8_t GetTileNumber() const { return tile_number_; }
	size_t GetObjPaletteNumber() const { return obj_palette_number_; }
	bool IsHorizontallyFlipped() const { return horizontal_flip_; }
	bool IsVerticallyFlipped() const { return vertical_flip_; }
	bool IsRenderedAboveBackground() const { return render_above_background_; }

	template<class Archive>
	void serialize(Archive &archive)
	{
		archive(y_, x_, tile_number_, obj_palette_number_, horizontal_flip_, vertical_flip_, render_above_background_);
	}

private:
	int y_{ -16 }; // A value of 0 in memory is translated to y = -16
	int x_{ -8 }; // A value of 0 in memory is translated to x = -8
	uint8_t tile_number_{ 0 };
	size_t obj_palette_number_{ 0 };
	bool horizontal_flip_{ false };
	bool vertical_flip_{ false };
	bool render_above_background_{ false };
};
