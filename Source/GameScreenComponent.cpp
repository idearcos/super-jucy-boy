#include "GameScreenComponent.h"
#include <string>

GameScreenComponent::GameScreenComponent() : OpenGl2DComponent{ 160, 144 }
{
	
}

GameScreenComponent::~GameScreenComponent()
{
	
}

void GameScreenComponent::OnNewFrame(const PPU::Framebuffer &ppu_framebuffer)
{
	std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };

	for (int i = 0; i < ppu_framebuffer.size(); ++i)
	{
		framebuffer_[i] = PpuColorToIntensity(ppu_framebuffer[i]);
	}

	openGLContext.triggerRepaint();
}

uint8_t GameScreenComponent::PpuColorToIntensity(PPU::Color color)
{
	switch (color)
	{
	case PPU::Color::White:
		return 255;
	case PPU::Color::LightGrey:
		return 192;
	case PPU::Color::DarkGrey:
		return 96;
	case PPU::Color::Black:
		return 0;
	default:
		throw std::invalid_argument("Invalid argument to PpuColorToIntensity: " + std::to_string(static_cast<int>(color)));
	}
}

void GameScreenComponent::render()
{
	const auto desktopScale = openGLContext.getRenderingScale();

	OpenGLHelpers::clear(Colour::greyLevel(0.1f));

	glViewport(0, 0, getWidth(), getHeight());

	shader_program_.use();

	// Bind and draw texture
	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	{std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_LUMINANCE, GL_UNSIGNED_BYTE, framebuffer_.data()); }

	// Bind VAO
	openGLContext.extensions.glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	openGLContext.extensions.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
