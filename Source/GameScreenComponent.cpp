#include "GL/glew.h"
#include "GameScreenComponent.h"
#include <cassert>

GameScreenComponent::GameScreenComponent() :
	vertices_{ Vertex{ { -1.0f, 1.0f },{ 0.0f, 0.0f } },
		Vertex{ { 1.0f, 1.0f },{ 1.0f, 0.0f } },
		Vertex{ { 1.0f, -1.0f },{ 1.0f, 1.0f } },
		Vertex{ { -1.0f, -1.0f },{ 0.0f, 1.0f } } },
	elements_{ 0, 1, 2, 2, 3, 0 },
	intensity_palette_{ 255, 192, 96, 0 }
{
	// It's important to set the OpenGL component painting to false, otherwise the OpenGL thread will need to lock Juce's MessageManager, which leads to all sorts of deadlocks
	openGLContext.setComponentPaintingEnabled(false);

	// Continuous repainting leads to greatly decreased performance (although disabling the above might solve this issue too, in case it was caused by locking the MessageManager continuously)
	openGLContext.setContinuousRepainting(false);

	framebuffer_.fill(255);
}

GameScreenComponent::~GameScreenComponent()
{
	shutdownOpenGL();
}

void GameScreenComponent::initialise()
{
	const auto glew_init_result = glewInit();
	if (glew_init_result != GLEW_OK)
	{
		juce::MessageManager::callAsync([glew_init_result]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "OpenGL error: failed to initialize GLEW",
				reinterpret_cast<const char*>(glewGetErrorString(glew_init_result)));
		});
		return;
	}

	if (!GLEW_VERSION_3_3)
	{
		juce::MessageManager::callAsync([]() {
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to initialize game screen component",
				"Minimum required OpenGL version: 3.3.\nVersion found: " + std::string{ reinterpret_cast<const char*>(glGetString(GL_VERSION)) });
		});
		return;
	}

#pragma region Shaders setup
	const GLchar* vertex_shader_source[]{
		"#version 330\n"
		"layout(location = 0) in vec2 vertex_position;\n"
		"layout(location = 1) in vec2 vertex_texcoord;\n"
		"out vec2 texcoord;\n"
		"void main() {\n"
		"  gl_Position = vec4 (vertex_position, 0.0, 1.0);\n"
		"  texcoord = vertex_texcoord;\n"
		"}\n"
	};

	const GLchar* fragment_shader_source[]{ GLEW_VERSION_4_2 ?
		"#version 420 core\n"
		"in vec2 texcoord;\n"
		"out vec4 frag_color;\n"
		"layout (binding = 0) uniform sampler2D tex;\n"
		"void main() {\n"
		"  frag_color = texture(tex, texcoord).rrra;\n"
		"}\n"
		:
		"#version 330\n"
		"in vec2 texcoord;\n"
		"out vec4 frag_color;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"  frag_color = texture(tex, texcoord).rrra;\n"
		"}\n"
	};

	// Create and compile vertex shader
	const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, vertex_shader_source, nullptr);
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
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to compile vertex shader", juce::String{ "Error: " } +error_message);
		});
	}

	// Create and compile fragment shader
	const auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, fragment_shader_source, nullptr);
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
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to compile fragment shader", juce::String{ "Error: " } +error_message);
		});
	}

	// Create program, attach shaders to it, and link it
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
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
			juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to link shader program", juce::String{ "Error: " } +error_message);
		});

		return;
	}

	// Delete the shaders as the program has them now
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (!GLEW_VERSION_4_2)
	{
		GLuint t1Location = glGetUniformLocation(shader_program_, "tex");
		glUniform1i(t1Location, 0);
	}

	glUseProgram(shader_program_);
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
	glBindTexture(GL_TEXTURE_2D, texture_);
	if (GLEW_VERSION_4_2)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification_filter_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Configure Position and Texture coordinate vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 2));

	// Attributes are disabled by default, therefore enable them
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Enable culling of back facing triangles (set CW winding order as front facing first)
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameScreenComponent::shutdown()
{
	glDeleteTextures(1, &texture_);

	glDeleteBuffers(1, &vertex_buffer_object_);
	glDeleteBuffers(1, &element_buffer_object_);
	glDeleteVertexArrays(1, &vertex_array_object_);

	glDeleteProgram(shader_program_);
}

void GameScreenComponent::OnNewFrame()
{
	std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };

	const auto ppu_framebuffer = ppu_->GetFramebuffer();

	for (int i = 0; i < ppu_framebuffer.size(); ++i)
	{
		assert(static_cast<size_t>(ppu_framebuffer[i]) < intensity_palette_.size());
		framebuffer_[i] = intensity_palette_[static_cast<size_t>(ppu_framebuffer[i])];
	}

	openGLContext.triggerRepaint();
}

void GameScreenComponent::SetPpu(PPU &ppu)
{
	ppu_ = &ppu;
	ppu_->AddListener([this]() { OnNewFrame(); });
}

void GameScreenComponent::UpdateFramebuffer()
{
	std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };
	const auto& ppu_framebuffer = ppu_->GetFramebuffer();

	for (int i = 0; i < ppu_framebuffer.size(); ++i)
	{
		assert(static_cast<size_t>(ppu_framebuffer[i]) < intensity_palette_.size());
		framebuffer_[i] = intensity_palette_[static_cast<size_t>(ppu_framebuffer[i])];
	}

	openGLContext.triggerRepaint();
}

void GameScreenComponent::render()
{
	const GLfloat bg_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, bg_color);

	glViewport(0, 0, getWidth(), getHeight());

	// Bind and draw texture
	glBindTexture(GL_TEXTURE_2D, texture_);
	{std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_RED, GL_UNSIGNED_BYTE, framebuffer_.data()); }

	// Bind VAO
	glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GameScreenComponent::SetMagnificationFilter(GLint magnification_filter)
{
	magnification_filter_ = magnification_filter;
	openGLContext.executeOnGLThread([this](juce::OpenGLContext&) {
		glBindTexture(GL_TEXTURE_2D, texture_);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification_filter_);
		glBindTexture(GL_TEXTURE_2D, 0);
	}, false);
}
