#include "../GL/glew.h"
#include "PpuDebugComponent.h"
#include <string>

PpuDebugComponent::PpuDebugComponent(DebugPPU &debug_ppu) :
	debug_ppu_{ &debug_ppu },
	vertices_{ InitializeVertices() },
	elements_{ InitializeElements() }
{
	openGLContext.setComponentPaintingEnabled(false);
	openGLContext.setContinuousRepainting(false);
	openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
}

PpuDebugComponent::~PpuDebugComponent()
{
	shutdownOpenGL();
}

std::vector<PpuDebugComponent::Vertex> PpuDebugComponent::InitializeVertices()
{
	std::vector<PpuDebugComponent::Vertex> vertices;

	// Tile height and width in OpenGL coordinates
	const auto opengl_tile_height = 2.0f / tile_grid_height_;
	const auto opengl_tile_width = 2.0f / tile_grid_width_;

	for (int jj = 0; jj < tile_grid_height_; ++jj)
	{
		const auto y_offset = +1.0f - opengl_tile_height * jj;

		for (int ii = 0; ii < tile_grid_width_; ++ii)
		{
			const auto x_offset = -1.0f + opengl_tile_width * ii;

			const auto layer = static_cast<float>(tile_grid_width_ * jj + ii);
			vertices.emplace_back(Vertex{ { x_offset, y_offset },{ 0.0f, 0.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset + opengl_tile_width, y_offset },{ 1.0f, 0.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset + opengl_tile_width, y_offset - opengl_tile_height },{ 1.0f, 1.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset, y_offset - opengl_tile_height },{ 0.0f, 1.0f, layer } });
		}
	}

	return vertices;
}

std::vector<GLuint> PpuDebugComponent::InitializeElements()
{
	std::vector<GLuint> elements;

	for (int ii = 0; ii < num_tiles_; ++ii)
	{
		elements.emplace_back(4 * ii);
		elements.emplace_back(4 * ii + 1);
		elements.emplace_back(4 * ii + 2);
		elements.emplace_back(4 * ii + 2);
		elements.emplace_back(4 * ii + 3);
		elements.emplace_back(4 * ii);
	}

	return elements;
}

void PpuDebugComponent::OnStatusUpdateRequested(bool /*compute_diff*/)
{
	std::unique_lock<std::mutex> lock{ tile_set_mutex_ };
	tile_set_ = debug_ppu_->GetTileSet();
	for (auto& tile : tile_set_)
	{
		std::transform(tile.begin(), tile.end(), tile.begin(), PpuColorNumberToIntensity);
	}

	openGLContext.triggerRepaint();
}

void PpuDebugComponent::initialise()
{
	const auto glew_init_result = glewInit();
	if (glew_init_result != GLEW_OK)
	{
		//TODO: do something
	}

#pragma region Shaders initialization
	const char* vertex_shader_source[]{
		"#version 400\n"
		"layout(location = 0) in vec2 vertex_position;\n"
		"layout(location = 1) in vec3 vertex_texcoord;\n"
		"out vec3 texcoord;\n"
		"void main() {\n"
		"  gl_Position = vec4 (vertex_position, 0.0, 1.0);\n"
		"  texcoord = vertex_texcoord;\n"
		"}\n" };

	const char* fragment_shader_source[]{
		"#version 400\n"
		"in vec3 texcoord;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2DArray textureArray;\n"
		"void main() {\n"
		"  frag_color = texture(textureArray, texcoord);\n"
		"}\n" };

	//Compile vertex shader
	vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_, 1, vertex_shader_source, nullptr);
	glCompileShader(vertex_shader_);

	GLint success = 0;
	glGetShaderiv(vertex_shader_, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success)
	{
		bool b = true;
	}

	//Compile fragment shader
	fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_, 1, fragment_shader_source, nullptr);
	glCompileShader(fragment_shader_);

	glGetShaderiv(fragment_shader_, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragment_shader_, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<char> infoLog(maxLength);
		glGetShaderInfoLog(fragment_shader_, maxLength, &maxLength, &infoLog[0]);
	}

	//Link vertex and fragment shader together
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader_);
	glAttachShader(shader_program_, fragment_shader_);
	glLinkProgram(shader_program_);

	GLint isLinked = 0;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		bool b = true;
	}

	glUseProgram(shader_program_);

	GLint texture_array_location = glGetUniformLocation(shader_program_, "textureArray");
	glUniform1i(texture_array_location, 0); //Sampler refers to texture unit 0

	//Delete shaders objects
	glDetachShader(shader_program_, vertex_shader_);
	glDetachShader(shader_program_, fragment_shader_);

	//TODO: what if something fails here? Juce catches and ignores the thrown exception...
#pragma endregion

	// Generate Vertex Array Object
	// The binding of the VAO must be done BEFORE binding the GL_ELEMENT_ARRAY_BUFFER
	// Otherwise the GL_ELEMENT_ARRAY_BUFFER won't be tied to the VAO state, and thus not automatically bound with it
	glGenVertexArrays(1, &vertex_array_object_);
	glBindVertexArray(vertex_array_object_);

	// Generate Vertex Buffer Object
	glGenBuffers(1, &vertex_buffer_object_);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(decltype(vertices_)::value_type), vertices_.data(), GL_STATIC_DRAW);

	// Generate Element Buffer Object
	glGenBuffers(1, &element_buffer_object_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements_.size() * sizeof(decltype(elements_)::value_type), elements_.data(), GL_STATIC_DRAW);

	// Generate texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, tile_width_, tile_height_, num_tiles_, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 2));

	// Attributes are disabled by default, therefore enable them
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void PpuDebugComponent::shutdown()
{
	glDeleteProgram(shader_program_);

	glDeleteShader(vertex_shader_);
	glDeleteShader(fragment_shader_);

	glDeleteBuffers(1, &vertex_buffer_object_);
	glDeleteBuffers(1, &element_buffer_object_);

	glDeleteTextures(1, &texture_);
}

void PpuDebugComponent::render()
{
	const auto desktopScale = openGLContext.getRenderingScale();

	OpenGLHelpers::clear(Colour::greyLevel(0.1f));

	glViewport(getWidth() / 2 - tile_grid_width_ * tile_width_, getHeight() / 2 - tile_grid_height_ * tile_height_, tile_grid_width_ * tile_width_ * 2, tile_grid_height_ * tile_height_ * 2);

	// Bind and draw texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	{std::unique_lock<std::mutex> lock{ tile_set_mutex_ };
	for (int ii = 0; ii < num_tiles_; ++ii)
	{
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, ii, tile_width_, tile_height_, 1, GL_LUMINANCE, GL_UNSIGNED_BYTE, tile_set_[ii].data());
	}}

	// Bind VAO
	glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	glBindVertexArray(0);
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
