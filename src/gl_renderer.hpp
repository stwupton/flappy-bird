#pragma once

#include <stdarg.h>

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "application.hpp"
#include "array.hpp"
#include "assets.hpp"
#include "game_properties.hpp"
#include "game_state.hpp"
#include "platform.hpp"

struct Basic_Shader_Program {
	GLuint id;
	struct {
		GLint view_projection;
		GLint transform;
	} uniform_location;
};

struct Shape_Shader_Program {
	GLuint id;
	struct {
		GLint shape_type;
		GLint colour;
		GLint view_projection;
		GLint transform;
	} uniform_location;
};

struct Shader_Contents {
	const char *vertex;
	const char *fragment;
};

struct GL_Renderer {
public:
	std::array<GLuint, static_cast<size_t>(Asset::Texture_ID::_length)> texture_indices = {};
	Basic_Shader_Program basic_shader_program;
	Shape_Shader_Program shape_shader_program;

private:
	Platform &platform;

public:
	GL_Renderer(Platform &platform) : platform{platform} {}

	void init(
		const Application &application,
		GLDEBUGPROC debug_message_handle, 
		const Shader_Contents &basic_shader_contents,
		const Shader_Contents &shape_shader_contents
	) {
		// Global settings
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debug_message_handle, 0);

		glEnable(GL_SCISSOR_TEST);

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);


		// Set up view projection transform and add it to the shader
		const float left = -((float)Game_Properties::view.width / 2);
		const float right = (float)Game_Properties::view.width / 2;
		const float bottom = -((float)Game_Properties::view.height / 2);
		const float top = (float)Game_Properties::view.height / 2;
		const glm::mat4 view_projection = glm::ortho(left, right, bottom, top);

		this->create_basic_shader(basic_shader_contents);
		glUniformMatrix4fv(this->basic_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);

		this->create_shape_shader(shape_shader_contents);
		glUniformMatrix4fv(this->shape_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);

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

		this->set_viewport(application);
	}

	void render(
		const Application &application, 
		const Game_State &state
	) const {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Basic renderer
		glUseProgram(this->basic_shader_program.id);
		Asset::Texture_ID cache_texture_id = Asset::Texture_ID::none;
		glm::mat4 identity = glm::mat4(1.0f);
		for (const Sprite &sprite : state.sprites) {
			const Asset::Texture &texture = Asset::get_texture(sprite.texture);

			if (sprite.texture != cache_texture_id) {
				glBindTexture(GL_TEXTURE_2D, this->texture_indices[(size_t)sprite.texture]);
				cache_texture_id = sprite.texture;
			}

			const glm::vec3 texture_size = glm::vec3(texture.width, texture.height, 1.0f);
			const glm::mat4 scale_transform = glm::scale(identity, texture_size);
			const glm::mat4 transform = sprite.transform * scale_transform;
			glUniformMatrix4fv(this->basic_shader_program.uniform_location.transform, 1, GL_FALSE, &transform[0][0]);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glUseProgram(this->shape_shader_program.id);
		for (const Shape &debug_shape : state.debug_shapes) {
			const glm::vec4 colour = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);

			glm::mat4 scale_transform;
			if (debug_shape.type == Shape_Type::rectangle) {
				scale_transform = glm::scale(identity, glm::vec3(debug_shape.rectangle.width, debug_shape.rectangle.height, 1.0f));
			} else if (debug_shape.type == Shape_Type::circle) {
				const float size = debug_shape.circle.radius * 2;
				scale_transform = glm::scale(identity, glm::vec3(size, size, 1.0f));
			}

			const glm::mat4 transform = debug_shape.transform * scale_transform;

			glUniform4fv(this->shape_shader_program.uniform_location.colour, 1, &debug_shape.colour[0]);
			glUniformMatrix4fv(this->shape_shader_program.uniform_location.transform, 1, GL_FALSE, &transform[0][0]);
			glUniform1i(this->shape_shader_program.uniform_location.shape_type, (GLint)debug_shape.type);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		glFlush();
	}

	void load(unsigned char *data, Asset::Texture_ID asset_id, const Asset::Texture &texture) {
		const size_t asset_index = static_cast<size_t>(asset_id);
		glBindTexture(GL_TEXTURE_2D, this->texture_indices[asset_index]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

private:
	void create_basic_shader(const Shader_Contents &contents) {
		// Compile shaders
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &contents.vertex, NULL);
		glCompileShader(vertex_shader);
		this->log_shader_compile_error(vertex_shader, "Basic Vertex");

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &contents.fragment, NULL);
		glCompileShader(fragment_shader);
		this->log_shader_compile_error(fragment_shader, "Basic Fragment");

		this->basic_shader_program.id = glCreateProgram();
		glAttachShader(this->basic_shader_program.id, vertex_shader);
		glAttachShader(this->basic_shader_program.id, fragment_shader);
		glLinkProgram(this->basic_shader_program.id);
		glUseProgram(this->basic_shader_program.id);
		this->log_shader_link_error(this->basic_shader_program.id, "Basic");

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		// Fetch uniform locations
		this->basic_shader_program.uniform_location.view_projection = glGetUniformLocation(this->basic_shader_program.id, "view_projection");
		this->basic_shader_program.uniform_location.transform = glGetUniformLocation(this->basic_shader_program.id, "transform");
	}

	void create_shape_shader(const Shader_Contents &contents) {
		// Compile shaders
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &contents.vertex, NULL);
		glCompileShader(vertex_shader);
		this->log_shader_compile_error(vertex_shader, "Shape Vertex");

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &contents.fragment, NULL);
		glCompileShader(fragment_shader);
		this->log_shader_compile_error(fragment_shader, "Shape Fragment");

		this->shape_shader_program.id = glCreateProgram();
		glAttachShader(this->shape_shader_program.id, vertex_shader);
		glAttachShader(this->shape_shader_program.id, fragment_shader);
		glLinkProgram(this->shape_shader_program.id);
		this->log_shader_link_error(this->shape_shader_program.id, "Shape");

		glUseProgram(this->shape_shader_program.id);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		// Fetch uniform locations
		this->shape_shader_program.uniform_location.view_projection = glGetUniformLocation(this->shape_shader_program.id, "view_projection");
		this->shape_shader_program.uniform_location.transform = glGetUniformLocation(this->shape_shader_program.id, "transform");
		this->shape_shader_program.uniform_location.colour = glGetUniformLocation(this->shape_shader_program.id, "colour");
		this->shape_shader_program.uniform_location.shape_type = glGetUniformLocation(this->shape_shader_program.id, "shape_type");
	}

	void set_viewport(const Application &application) {
		const float window_aspect_ratio = (float)application.window.width / application.window.height;

		// TODO(steven): Define the game screen size somewhere else
		const float view_aspect_ratio = (float)Game_Properties::view.width / Game_Properties::view.height;
		
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
	}

	void log(const char *format, ...) const {
		va_list args;
		va_start(args, format);
		const int message_length = vsnprintf(NULL, 0, format, args);
		const int message_byte_length = sizeof(char) * (message_length + 1);
		assert(message_length >= 0);
		char *message = (char *)malloc(message_byte_length);
		memset(message, 0, message_byte_length);
		vsnprintf(message, message_byte_length, format, args);
		va_end(args);

		this->platform.log_info("GL Log: %s", message);
		free(message);
	}

	void log_shader_compile_error(GLuint shader_id, const char *name) const {
		GLint success;
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			GLint message_length;
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &message_length);

			GLint message_byte_length = sizeof(char) * message_length;
			char *message = (char *)malloc(message_byte_length);
			memset(message, 0, message_byte_length);
			glGetShaderInfoLog(shader_id, message_length, NULL, message);

			this->log("Shader (%s) failed to compile. Message: %s", name, message);
			free(message);
		}
	}

	void log_shader_link_error(GLuint program_id, const char *name) const {
		GLint success;
		glGetProgramiv(program_id, GL_LINK_STATUS, &success);

		if (success == GL_FALSE) {
			GLint message_length;
			glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &message_length);

			GLint message_byte_length = sizeof(char) * message_length;
			char *message = (char *)malloc(message_byte_length);
			memset(message, 0, message_byte_length);
			glGetProgramInfoLog(program_id, message_length, NULL, message);

			this->log("Program (%s) failed to link. Message: %s", name, message);
			free(message);
		}
	}
};