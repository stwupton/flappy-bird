#include "gl_renderer.hpp"

void GL_Renderer::init(
	DEBUG_MESSAGE_CALLBACK debug_message_handle, 
	const char *vertex_contents, 
	const char *fragment_contents
) {
	// Global settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(debug_message_handle, 0);

	glEnable(GL_SCISSOR_TEST);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

	// Compile shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_contents, NULL);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_contents, NULL);
	glCompileShader(fragment_shader);

	this->shader_program = glCreateProgram();
	glAttachShader(this->shader_program, vertex_shader);
	glAttachShader(this->shader_program, fragment_shader);
	glLinkProgram(this->shader_program);
	glUseProgram(this->shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	// Generate all texture indices
	glGenTextures(static_cast<GLsizei>(this->texture_indices.size()), this->texture_indices.data());

	// Create vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create buffer array object
	// Data: x, y, z, s, t
	// Must draw counter clockwise to face forward
	const float vertices[] = {
		-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Setup vertex attributes.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void GL_Renderer::render(
	const Application &application, 
	const Game_State &state
) const {
	const float window_aspect_ratio = (float)application.window.width / application.window.height;

	// TODO(steven): Define the game screen size somewhere else
	const float view_aspect_ratio = (float)Game_Properties::view_width / Game_Properties::view_height;
	
	GLsizei viewport_height;
	GLsizei viewport_width;

	const bool snap_to_height = window_aspect_ratio > view_aspect_ratio;
	if (snap_to_height) {
		viewport_height = application.window.height;
		viewport_width = (GLsizei)(viewport_height * view_aspect_ratio);
	} else {
		viewport_width = application.window.width;
		viewport_height = (GLsizei)(viewport_width / view_aspect_ratio);
	}

	const GLsizei viewport_x = application.window.width / 2 - viewport_width / 2;
	const GLsizei viewport_y = application.window.height / 2 - viewport_height / 2;

	glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
	glScissor(viewport_x, viewport_y, viewport_width, viewport_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const float left = -((float)Game_Properties::view_width / 2);
	const float right = (float)Game_Properties::view_width / 2;
	const float bottom = -((float)Game_Properties::view_height / 2);
	const float top = (float)Game_Properties::view_height / 2;
	const glm::mat4 view_projection = glm::ortho(left, right, bottom, top);

	const GLuint view_projection_uniform_location = glGetUniformLocation(this->shader_program, "view_projection");
	glUniformMatrix4fv(view_projection_uniform_location, 1, GL_FALSE, &view_projection[0][0]);

	const GLuint texture_size_uniform_location = glGetUniformLocation(this->shader_program, "texture_size");
	const GLuint transform_uniform_location = glGetUniformLocation(this->shader_program, "transform");

	for (const Sprite &sprite : state.sprites) {
		const Asset::Texture &texture = Asset::get_texture(sprite.texture);

		glBindTexture(GL_TEXTURE_2D, this->texture_indices[(size_t)sprite.texture]);

		const glm::vec2 texture_size = glm::vec2(texture.width, texture.height);
		glUniform2fv(texture_size_uniform_location, 1, &texture_size[0]);

		glUniformMatrix4fv(transform_uniform_location, 1, GL_FALSE, &sprite.transform[0][0]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glFlush();
}

void GL_Renderer::load(unsigned char *data, Asset::Texture_ID asset_id, const Asset::Texture &texture) {
	const size_t asset_index = static_cast<size_t>(asset_id);
	glBindTexture(GL_TEXTURE_2D, this->texture_indices[asset_index]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}