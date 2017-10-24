#pragma once

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../JucyBoy/PPU.h"

class TilesetComponent final : public juce::OpenGLAppComponent
{
public:
	TilesetComponent();
	~TilesetComponent();

	void SetPpu(PPU* ppu) { ppu_ = ppu; }

	void UpdateTileSet();

	// juce::OpenGLAppComponent overrides
	void render();
	void initialise();
	void shutdown();

	// juce::Component overrides
	void resized() override;

private:
	struct Vertex
	{
		float position[2];
		float texCoord[3];
	};

	static std::vector<Vertex> InitializeVertices();
	static std::vector<GLuint> InitializeElements();

public:
	static constexpr size_t tile_grid_width_{ 16 };
	static constexpr size_t tile_grid_height_{ 24 };
	static constexpr size_t num_tiles_{ tile_grid_width_ * tile_grid_height_ };
	static constexpr size_t tile_width_{ 8 };
	static constexpr size_t tile_height_{ 8 };

private:
	PPU::Tileset tile_set_{};
	std::atomic<bool> update_sync_;

	// OpenGL stuff
	GLuint vertex_array_object_{ 0 };
	GLuint vertex_buffer_object_{ 0 };
	GLuint element_buffer_object_{ 0 };
	GLuint shader_program_{ 0 };
	GLuint texture_{ 0 };

	const std::vector<Vertex> vertices_;
	const std::vector<GLuint> elements_;

	std::array<uint8_t, static_cast<size_t>(PPU::Color::Count)> intensity_palette_;

	juce::Rectangle<int> viewport_area_;
	bool opengl_initialization_complete_{ false };

	PPU* ppu_{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TilesetComponent)
};
