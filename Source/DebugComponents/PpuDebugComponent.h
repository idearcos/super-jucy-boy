#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JucyBoy/PPU.h"

class PpuTileSetComponent final : public juce::Component
{
public:
	PpuTileSetComponent();
	~PpuTileSetComponent() = default;

	void SetPpu(PPU* ppu) { ppu_ = ppu; }

	void UpdateTileSet();

	void render();
	void initialise();
	void shutdown();

private:
	struct Vertex
	{
		float position[2];
		float texCoord[3];
	};

	static std::vector<Vertex> InitializeVertices();
	static std::vector<GLuint> InitializeElements();

public:
	static constexpr size_t num_tiles_{ 384 };
	static constexpr size_t tile_grid_width_{ 16 };
	static constexpr size_t tile_grid_height_{ 24 };
	static constexpr size_t tile_width_{ 8 };
	static constexpr size_t tile_height_{ 8 };
	static_assert(tile_grid_width_ * tile_grid_height_ == num_tiles_, "Invalid number of tile rows and columns");

private:
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

	std::array<uint8_t, static_cast<size_t>(PPU::Color::Count)> intensity_palette_;

	PPU* ppu_{ nullptr };
};

class PpuDebugComponent final : public juce::OpenGLAppComponent, public PPU::Listener
{
public:
	PpuDebugComponent();
	~PpuDebugComponent();

	void SetPpu(PPU* ppu);

	void Update();

	// juce::OpenGLAppComponent overrides
	void render() override;
	void initialise() override;
	void shutdown() override;

	// juce::Component overrides
	void paint(juce::Graphics&) override {}
	void resized() override;

private:
	PpuTileSetComponent tileset_component_;

	bool opengl_initialization_complete_{ false };

	PPU* ppu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PpuDebugComponent)
};
