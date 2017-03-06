#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdint>
#include <array>
#include "JucyBoy/PPU.h"

class GameScreenComponent final : public juce::OpenGLAppComponent, public PPU::Listener
{
public:
	GameScreenComponent();
	~GameScreenComponent();

	void initialise() override;
	void shutdown() override;
	void render() override;

	void paint (Graphics&) override;
	void resized() override;

	void OnNewFrame(const PPU::Framebuffer &ppu_framebuffer) override;

private:
	uint8_t PpuColorToIntensity(PPU::Color color);

private:
	struct Vertex
	{
		float position[2];  // To define vertex x,y,z coordinates
		float colour[3];    // Color used for the vertex. If no other color info is given for the fragment
							// the pixel colors will be interpolated from the vertex colors
		float texCoord[2];  // A graphic image (file) can be used to define the texture of the drawn object.
							// This 2-D vector gives the coordinates in the 2-D image file corresponding to
							// the pixel color to be drawn
	};

	GLuint vertex_array_object_{ 0 };
	GLuint vertex_buffer_object_{ 0 };
	GLuint element_buffer_object_{ 0 };
	juce::OpenGLShaderProgram shader_program{ openGLContext };
	GLuint texture_{ 0 };

	const std::vector<Vertex> vertices_;
	const std::vector<GLuint> elements_;

	std::array<uint8_t, 160 * 144 * 3> framebuffer_;
	std::mutex framebuffer_mutex_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GameScreenComponent)
};
