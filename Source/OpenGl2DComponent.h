#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OpenGl2DComponent : public juce::OpenGLAppComponent
{
public:
	OpenGl2DComponent(size_t width, size_t height);
	~OpenGl2DComponent();

	// juce::OpenGLAppComponent overrides
	void initialise() override final;
	void shutdown() override final;
	virtual void render() override = 0;

protected:
	struct Vertex
	{
		float position[2];	// To define vertex x,y,z coordinates
		float texCoord[2];	// A graphic image (file) can be used to define the texture of the drawn object.
							// This 2-D vector gives the coordinates in the 2-D image file corresponding to
							// the pixel color to be drawn
	};

	GLuint vertex_array_object_{ 0 };
	GLuint vertex_buffer_object_{ 0 };
	GLuint element_buffer_object_{ 0 };
	juce::OpenGLShaderProgram shader_program_{ openGLContext };
	GLuint texture_{ 0 };

	const std::vector<Vertex> vertices_;
	const std::vector<GLuint> elements_;

private:
	size_t width_{ 0 };
	size_t height_{ 0 };

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGl2DComponent)
};
