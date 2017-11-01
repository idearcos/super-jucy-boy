#include "GL/glew.h"
#include "BackgroundRenderer.h"
#include <cassert>
#include <string>

BackgroundRenderer::BackgroundRenderer() :
	vertices_{ InitializeVertices() },
	elements_{ InitializeElements() },
	intensity_palette_{ 255, 192, 96, 0 }
{
	for (auto& tile : tile_set_)
	{
		tile.fill(255);
	}

	tile_map_.fill(0);
}

std::vector<BackgroundRenderer::Vertex> BackgroundRenderer::InitializeVertices()
{
	std::vector<Vertex> vertices;

	// Tile width and height in OpenGL coordinates
	const auto opengl_tile_width = 2.0f / bg_width_in_tiles_;
	const auto opengl_tile_height = 2.0f / bg_height_in_tiles_;

	for (int jj = 0; jj < bg_height_in_tiles_; ++jj)
	{
		const auto y_offset = +1.0f - opengl_tile_height * jj;

		for (int ii = 0; ii < bg_width_in_tiles_; ++ii)
		{
			const auto x_offset = -1.0f + opengl_tile_width * ii;

			const auto layer = static_cast<float>(bg_width_in_tiles_ * jj + ii);
			vertices.emplace_back(Vertex{ { x_offset, y_offset },{ 0.0f, 0.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset + opengl_tile_width, y_offset },{ 1.0f, 0.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset + opengl_tile_width, y_offset - opengl_tile_height },{ 1.0f, 1.0f, layer } });
			vertices.emplace_back(Vertex{ { x_offset, y_offset - opengl_tile_height },{ 0.0f, 1.0f, layer } });
		}
	}

	return vertices;
}

std::vector<GLuint> BackgroundRenderer::InitializeElements()
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

void BackgroundRenderer::initialise()
{
	const auto glew_init_result = glewInit();
	if (glew_init_result != GLEW_OK)
	{
		std::string glew_error{ reinterpret_cast<const char*>(glewGetErrorString(glew_init_result)) };
		juce::MessageManager::callAsync([glew_error = std::move(glew_error)]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "OpenGL error: failed to initialize GLEW", glew_error);
			juce::JUCEApplicationBase::quit();
		});
		return;
	}

	if (!GLEW_VERSION_3_0)
	{
		std::string opengl_version{ reinterpret_cast<const char*>(glGetString(GL_VERSION)) };
		juce::MessageManager::callAsync([opengl_version = std::move(opengl_version)]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to initialize PPU debug component",
				"Minimum required OpenGL version: 3.0.\nVersion found: " + opengl_version);
			juce::JUCEApplicationBase::quit();
		});
		return;
	}

#pragma region Shaders setup
	// Explicit attribute locations are supported from GLSL 330
	std::basic_string<GLchar> vertex_shader_source_string;
	vertex_shader_source_string += GLEW_VERSION_4_2 ? "#version 420 core\n" : (GLEW_VERSION_3_3 ? "#version 330 core\n" : "#version 130\n");
	vertex_shader_source_string += GLEW_VERSION_3_3 ? "layout(location = 0) in vec2 vertex_position;\n" : "in vec2 vertex_position;\n";
	vertex_shader_source_string += GLEW_VERSION_3_3 ? "layout(location = 1) in vec3 vertex_texcoord;\n" : "in vec3 vertex_texcoord;\n";
	vertex_shader_source_string += "out vec3 texcoord;\n";
	vertex_shader_source_string += "void main() {\n";
	vertex_shader_source_string += "  gl_Position = vec4 (vertex_position, 0.0, 1.0);\n";
	vertex_shader_source_string += "  texcoord = vertex_texcoord;\n";
	vertex_shader_source_string += "}\n";

	// Create and compile vertex shader
	const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const auto vertex_shader_source = vertex_shader_source_string.c_str();
	glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
	glCompileShader(vertex_shader);

	GLint success = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &infoLog[0]);

		std::string error_message(infoLog.data(), maxLength);

		juce::MessageManager::callAsync([error_message = std::move(error_message)]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to compile vertex shader", "Error: " + error_message);
		});
	}

	// Binding points in layout qualifier are supported from GLSL 420
	std::basic_string<GLchar> fragment_shader_source_string;
	fragment_shader_source_string += GLEW_VERSION_4_2 ? "#version 420 core\n" : (GLEW_VERSION_3_3 ? "#version 330 core\n" : "#version 130\n");
	fragment_shader_source_string += "in vec3 texcoord;\n";
	fragment_shader_source_string += "out vec4 frag_color;\n";
	fragment_shader_source_string += GLEW_VERSION_4_2 ? "layout (binding = 0) uniform sampler2DArray tex;\n" : "uniform sampler2DArray tex;\n";
	fragment_shader_source_string += "void main() {\n";
	fragment_shader_source_string += "  frag_color = texture(tex, texcoord).rrra;\n";
	fragment_shader_source_string += "}\n";

	// Create and compile fragment shader
	const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const auto fragment_shader_source = fragment_shader_source_string.c_str();
	glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &infoLog[0]);

		std::string error_message(infoLog.data(), maxLength);

		juce::MessageManager::callAsync([error_message = std::move(error_message)]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to compile fragment shader", "Error: " + error_message);
		});
	}

	// Create program, attach shaders to it
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, fragment_shader);

	// Perform additional pre-link operations
	if (!GLEW_VERSION_3_3)
	{
		glBindAttribLocation(shader_program_, 0, "vertex_position");
		glBindAttribLocation(shader_program_, 1, "vertex_texcoord");
	}

	// Link program
	glLinkProgram(shader_program_);

	GLint isLinked = 0;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(shader_program_, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shader_program_, maxLength, &maxLength, &infoLog[0]);

		std::string error_message(infoLog.data(), maxLength);

		juce::MessageManager::callAsync([error_message = std::move(error_message)]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to link shader program", "Error: " + error_message);
		});
	}

	// Delete the shaders as the program has them now
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Bind texture sampler to texture unit 0, if layout binding points are not supported
	if (!GLEW_VERSION_4_2)
	{
		glUniform1i(glGetUniformLocation(shader_program_, "tex"), 0);
	}

	glUseProgram(shader_program_);
#pragma endregion

	// Generate Vertex Array Object
	// The binding of the VAO must be done BEFORE binding the GL_ELEMENT_ARRAY_BUFFER
	// Otherwise the GL_ELEMENT_ARRAY_BUFFER won't be tied to the VAO state, and thus not automatically bound with it
	glGenVertexArrays(1, &vertex_array_object_);
	GLenum e = glGetError();
	glBindVertexArray(vertex_array_object_);
	e = glGetError();

	// Generate Vertex Buffer Object
	glGenBuffers(1, &vertex_buffer_object_);
	e = glGetError();
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
	e = glGetError();
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(decltype(vertices_)::value_type), vertices_.data(), GL_STATIC_DRAW);
	e = glGetError();

	// Generate Element Buffer Object
	glGenBuffers(1, &element_buffer_object_);
	e = glGetError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object_);
	e = glGetError();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements_.size() * sizeof(decltype(elements_)::value_type), elements_.data(), GL_STATIC_DRAW);
	e = glGetError();

	// Generate texture
	glActiveTexture(GL_TEXTURE0);
	e = glGetError();
	glGenTextures(1, &texture_);
	e = glGetError();
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_);
	e = glGetError();
	if (GLEW_VERSION_4_2)
	{
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGB8, tile_width_, tile_height_, num_tiles_);
	}
	else
	{
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, tile_width_, tile_height_, num_tiles_, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
	e = glGetError();

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	e = glGetError();

	// Configure Position and Texture coordinate vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 2));
	e = glGetError();

	// Attributes are disabled by default, therefore enable them
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	e = glGetError();

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	e = glGetError();

	opengl_initialization_complete_ = true;
}

void BackgroundRenderer::shutdown()
{
	glDeleteTextures(1, &texture_);

	glDeleteBuffers(1, &vertex_buffer_object_);
	glDeleteBuffers(1, &element_buffer_object_);
	glDeleteVertexArrays(1, &vertex_array_object_);

	glDeleteProgram(shader_program_);
}

void BackgroundRenderer::render()
{
	if (!opengl_initialization_complete_) return;

	const GLfloat bg_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, bg_color);

	glViewport(viewport_area_.getX(), viewport_area_.getY(), viewport_area_.getWidth(), viewport_area_.getHeight());

	// Bind and draw texture
	glBindTexture(GL_TEXTURE_2D, texture_);
	const auto updated = update_sync_.load(std::memory_order::memory_order_acquire);
	for (int ii = 0; ii < num_tiles_; ++ii)
	{
		const auto tile_index = active_tile_set_ ? tile_map_[ii] : 256 + static_cast<int8_t>(tile_map_[ii]);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, ii, tile_width_, tile_height_, 1, GL_RED, GL_UNSIGNED_BYTE, tile_set_[tile_index].data());
	}

	// Bind VAO
	glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void BackgroundRenderer::Update()
{
	if (!ppu_) return;

	const auto& ppu_tile_set = ppu_->GetTileSet();
	assert(ppu_tile_set.size() == tile_set_.size());
	for (auto ii = 0; ii < ppu_tile_set.size(); ++ii)
	{
		std::transform(ppu_tile_set[ii].begin(), ppu_tile_set[ii].end(), tile_set_[ii].begin(), [this](uint8_t color_number) { return intensity_palette_[color_number]; });
	}

	tile_map_ = ppu_->GetTileMap(selected_tile_map_.value_or(ppu_->GetDetectedActiveBackgroundTileMap()));

	active_tile_set_ = ppu_->GetActiveTileSet();

	update_sync_.store(true, std::memory_order::memory_order_release);
}

void BackgroundRenderer::SetViewportArea(const juce::Rectangle<int> &viewport_area)
{
	const auto side_length = std::min(viewport_area.getWidth(), viewport_area.getHeight());
	viewport_area_ = juce::Rectangle<int>((viewport_area.getWidth() - side_length) / 2, (viewport_area.getHeight() - side_length) / 2, side_length, side_length);
}

void BackgroundRenderer::SetSelectedTileMap(std::optional<size_t> selected_tile_map)
{
	selected_tile_map_ = selected_tile_map;
	Update();
}
