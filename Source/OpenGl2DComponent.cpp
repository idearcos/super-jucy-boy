#include "OpenGl2DComponent.h"

OpenGl2DComponent::OpenGl2DComponent(size_t width, size_t height) :
	width_{ width },
	height_{ height },
	vertices_{ Vertex{ { -1.0f, 1.0f },{ 0.0f, 0.0f } },
		Vertex{ { 1.0f, 1.0f },{ 1.0f, 0.0f } },
		Vertex{ { 1.0f, -1.0f },{ 1.0f, 1.0f } },
		Vertex{ { -1.0f, -1.0f },{ 0.0f, 1.0f } } },
	elements_{ 0, 1, 2, 2, 3, 0 }
{
	// It's important to set the OpenGL component painting to false, otherwise the OpenGL thread will need to lock Juce's MessageManager, which leads to all sorts of deadlocks
	openGLContext.setComponentPaintingEnabled(false);

	// Continuous repainting leads to greatly decreased performance (although disabling the above might solve this issue too, in case it was caused by locking the MessageManager continuously)
	openGLContext.setContinuousRepainting(false);

	openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
}

OpenGl2DComponent::~OpenGl2DComponent()
{
	shutdownOpenGL();
}

void OpenGl2DComponent::initialise()
{
#pragma region Shaders initialization
	const std::string vertex_shader{
		"#version 400\n"
		"layout(location = 0) in vec2 vertex_position;\n"
		"layout(location = 1) in vec2 vertex_texcoord;\n"
		"out vec2 texcoord;\n"
		"void main() {\n"
		"  gl_Position = vec4 (vertex_position, 0.0, 1.0);\n"
		"  texcoord = vertex_texcoord;\n"
		"}\n" };

	const std::string fragment_shader{
		"#version 400\n"
		"in vec2 texcoord;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"  frag_color = texture2D(tex, texcoord);\n"
		"}\n" };

	if (!shader_program_.addVertexShader(vertex_shader)
		|| !shader_program_.addFragmentShader(fragment_shader)
		|| !shader_program_.link())
	{
		const auto error = shader_program_.getLastError();
		throw std::runtime_error{ std::string{ "Failed to create OpenGL shaders: " } + error.toStdString() };
	}
#pragma endregion

	// Generate Vertex Array Object
	// The binding of the VAO must be done BEFORE binding the GL_ELEMENT_ARRAY_BUFFER
	// Otherwise the GL_ELEMENT_ARRAY_BUFFER won't be tied to the VAO state, and thus not automatically bound with it
	openGLContext.extensions.glGenVertexArrays(1, &vertex_array_object_);
	openGLContext.extensions.glBindVertexArray(vertex_array_object_);

	// Generate Vertex Buffer Object
	openGLContext.extensions.glGenBuffers(1, &vertex_buffer_object_);
	openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
	openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(decltype(vertices_)::value_type), vertices_.data(), GL_STATIC_DRAW);

	// Generate Element Buffer Object
	openGLContext.extensions.glGenBuffers(1, &element_buffer_object_);
	openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_);
	openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements_.size() * sizeof(decltype(elements_)::value_type), elements_.data(), GL_STATIC_DRAW);

	// Generate texture
	openGLContext.extensions.glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

	openGLContext.extensions.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	openGLContext.extensions.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 2));

	// Attributes are disabled by default, therefore enable them
	openGLContext.extensions.glEnableVertexAttribArray(0);
	openGLContext.extensions.glEnableVertexAttribArray(1);

	openGLContext.extensions.glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGl2DComponent::shutdown()
{
	openGLContext.extensions.glDeleteBuffers(1, &vertex_buffer_object_);
	openGLContext.extensions.glDeleteBuffers(1, &element_buffer_object_);

	glDeleteTextures(1, &texture_);
}
