#include "PpuDebugComponent.h"
#include <string>

PpuDebugComponent::PpuDebugComponent(DebugPPU &debug_ppu) : OpenGl2DComponent{ num_tile_columns_ * tile_width_, num_tile_rows_ * tile_height_ },
	debug_ppu_{ &debug_ppu }
{

}

PpuDebugComponent::~PpuDebugComponent()
{
}

void PpuDebugComponent::OnStatusUpdateRequested(bool /*compute_diff*/)
{
	tile_set_ = debug_ppu_->GetTileSet();

	for (int tile_index = 0; tile_index < tile_set_.size(); ++tile_index)
	{
		for (int y = 0; y < tile_height_; ++y)
		{
			for (int x = 0; x < tile_width_; ++x)
			{
				const auto tile_row = tile_index / num_tile_columns_;
				const auto tile_column = tile_index % num_tile_columns_;
				framebuffer_[(tile_row * tile_height_ + y) * num_tile_columns_ * tile_width_ + tile_column * tile_width_ + x] = PpuColorNumberToIntensity(tile_set_[tile_index][y * tile_width_ + x]);
			}
		}
	}

	openGLContext.triggerRepaint();
}

void PpuDebugComponent::render()
{
	const auto desktopScale = openGLContext.getRenderingScale();

	OpenGLHelpers::clear(Colour::greyLevel(0.1f));

	glViewport(getWidth() / 2 - num_tile_columns_ * tile_width_, getHeight() / 2 - num_tile_rows_ * tile_height_, num_tile_columns_ * tile_width_ * 2, num_tile_rows_ * tile_height_ * 2);

	shader_program_.use();

	// Bind and draw texture
	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	{std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, num_tile_columns_ * tile_width_, num_tile_rows_ * tile_height_, GL_LUMINANCE, GL_UNSIGNED_BYTE, framebuffer_.data()); }

	// Bind VAO
	openGLContext.extensions.glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	openGLContext.extensions.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

uint8_t PpuDebugComponent::PpuColorNumberToIntensity(uint8_t color_number)
{
	switch (color_number)
	{
	case 0:
		return 255;
	case 1:
		return 192;
	case 2:
		return 96;
	case 3:
		return 0;
	default:
		throw std::invalid_argument("Invalid argument to PpuColorNumberToIntensity: " + std::to_string(static_cast<int>(color_number)));
	}
}
