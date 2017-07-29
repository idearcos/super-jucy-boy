#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/Debug/DebugPPU.h"

class PpuDebugComponent final : public juce::OpenGLAppComponent
{
public:
	PpuDebugComponent();
	~PpuDebugComponent();

	void SetPpu(DebugPPU &debug_ppu) { debug_ppu_ = &debug_ppu; }

	void UpdateState();

	// OpenGl2DComponent overrides
	void render() override;
	void initialise() override;
	void shutdown() override;

	// juce::Component overrides
	void paint(juce::Graphics&) override {}
	void resized() override {}

private:
	struct Vertex
	{
		float position[2];
		float texCoord[3];
	};

	static GLuint CompileShaderProgram();
	static std::vector<Vertex> InitializeVertices();
	static std::vector<GLuint> InitializeElements();
	static uint8_t PpuColorNumberToIntensity(uint8_t color_number);

private:
	static constexpr size_t num_tiles_{ 384 };
	static constexpr size_t tile_grid_width_{ 16 };
	static constexpr size_t tile_grid_height_{ 24 };
	static constexpr size_t tile_width_{ 8 };
	static constexpr size_t tile_height_{ 8 };
	static_assert(tile_grid_width_ * tile_grid_height_ == num_tiles_, "Invalid number of tile rows and columns");

	std::array<PPU::Tile, num_tiles_> tile_set_{};
	std::mutex tile_set_mutex_;

	// OpenGL stuff
	GLuint vertex_array_object_{ 0 };
	GLuint vertex_buffer_object_{ 0 };
	GLuint element_buffer_object_{ 0 };
	GLuint shader_program_{ 0 };
	GLuint texture_{ 0 };

	const std::vector<Vertex> vertices_;
	const std::vector<GLuint> elements_;

	DebugPPU* debug_ppu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PpuDebugComponent)
};
