#include "GL/glew.h"
#include "GameScreenComponent.h"
#include <string>

GameScreenComponent::GameScreenComponent() :
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

	//TODO: more proper version checking using OpenGL directly
	openGLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);

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
		//TODO: do something
	}

	shader_program_ = CompileShaderProgram();

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
	if (GLEW_VERSION_4_3)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
		//glTexSubImage2D(texture_, 0, 0, 0,  static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

GLuint GameScreenComponent::CompileShaderProgram()
{
	const ::GLchar* vertex_shader_source[]{
		"#version 430 core\n"
		"layout(location = 0) in vec2 vertex_position;\n"
		"layout(location = 1) in vec2 vertex_texcoord;\n"
		"out vec2 texcoord;\n"
		"void main() {\n"
		"  gl_Position = vec4 (vertex_position, 0.0, 1.0);\n"
		"  texcoord = vertex_texcoord;\n"
		"}\n" };

	const ::GLchar* fragment_shader_source[]{
		"#version 430 core\n"
		"in vec2 texcoord;\n"
		"out vec4 frag_color;\n"
		"layout (binding = 0) uniform sampler2D tex;\n"
		"void main() {\n"
		"  frag_color = texture2D(tex, texcoord);\n"
		"}\n" };

	// Create and compile vertex shader
	const auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, vertex_shader_source, nullptr);
	glCompileShader(vertex_shader);

	GLint success = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (GL_FALSE == success)
	{
		bool b = true;
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
		std::vector<char> infoLog(maxLength);
		glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &infoLog[0]);
	}

	// Create program, attach shaders to it, and link it
	const auto shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	GLint isLinked = 0;
	glGetProgramiv(shader_program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		bool b = true;
	}

	// Delete the shaders as the program has them now
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader_program;
}

void GameScreenComponent::shutdown()
{
	glDeleteTextures(1, &texture_);

	glDeleteBuffers(1, &vertex_buffer_object_);
	glDeleteBuffers(1, &element_buffer_object_);
	glDeleteVertexArrays(1, &vertex_array_object_);

	glDeleteProgram(shader_program_);
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

void GameScreenComponent::render()
{
	const auto desktopScale = openGLContext.getRenderingScale();

	const GLfloat bg_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, bg_color);

	glViewport(0, 0, getWidth(), getHeight());

	glUseProgram(shader_program_);

	// Bind and draw texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	{std::unique_lock<std::mutex> lock{ framebuffer_mutex_ };
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 160, 144, GL_LUMINANCE, GL_UNSIGNED_BYTE, framebuffer_.data()); }

	// Bind VAO
	glBindVertexArray(vertex_array_object_);

	// Draw triangles
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(elements_.size()), GL_UNSIGNED_INT, 0);

	// Unbind VAO and texture
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
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
