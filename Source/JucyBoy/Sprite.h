#pragma once

#include <cstdint>

class Sprite final
{
public:
	Sprite() = default;
	~Sprite() = default;

private:
	uint8_t y_{ 0 };
	uint8_t x_{ 0 };
	uint8_t tile_{ 0 };
	size_t palette_{ 0 };
	bool horizontal_flip_{ false };
	bool vertical_flip_{ false };
	bool render_above_background_{ false };
};
