#pragma once

#include <stdarg.h>
#include <string>

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

struct Text_Shader_Program {
	GLuint id;
	struct {
		GLint text_colour;
		GLint view_projection;
		GLint transform;
	} uniform_location;
};

struct Shader_Contents {
	const char *vertex;
	const char *fragment;
};


struct Font_Face_Character {
	GLuint texture_id;
	int width, height;
	int left, top;
	int advance_x;
};

struct Font_Face {
	int height;
	std::array<Font_Face_Character, 128> characters;
};

struct GL_Renderer {
public:
	std::array<GLuint, static_cast<size_t>(Asset::Texture_ID::_length)> texture_indices = {};
	Font_Face font_face;
	Basic_Shader_Program basic_shader_program;
	Shape_Shader_Program shape_shader_program;
	Text_Shader_Program text_shader_program;
	GLuint generic_vao;
	GLuint text_vao;

private:
	Platform &platform;
	Size<int> cached_window_size;

public:
	GL_Renderer(Platform &platform) : platform{platform} {}

	void init(
		// TODO(steven): remove application
		const Application &application,
		GLDEBUGPROC debug_message_handle, 
		const Shader_Contents &basic_shader_contents,
		const Shader_Contents &shape_shader_contents,
		const Shader_Contents &text_shader_contents
	) {
		// Global settings
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debug_message_handle, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

		this->create_text_shader(text_shader_contents);
		glUniformMatrix4fv(this->text_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);

		// Generate all texture indices
		glGenTextures(static_cast<GLsizei>(this->texture_indices.size()), this->texture_indices.data());

		// Create generic vertex array object
		glGenVertexArrays(1, &this->generic_vao);
		glBindVertexArray(this->generic_vao);

		// Create buffer array object
		// Data: x, y, z, s, t
		// Must draw counter clockwise to face forward
		const float generic_vertices[] = {
			-0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
		};

		GLuint generic_vbo;
		glGenBuffers(1, &generic_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, generic_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(generic_vertices), generic_vertices, GL_STATIC_DRAW);

		// Setup generic vertex attributes.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		// Create generic vertex array object
		glGenVertexArrays(1, &this->text_vao);
		glBindVertexArray(this->text_vao);

		const float text_vertices[] = {
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};

		GLuint text_vbo;
		glGenBuffers(1, &text_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(text_vertices), text_vertices, GL_STATIC_DRAW);

		// Setup text vertex attributes.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	void render(
		const Application &application, 
		const Game_State &state
	) {
		this->set_viewport(application);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 identity = glm::identity<glm::mat4>();

		// Basic renderer
		glUseProgram(this->basic_shader_program.id);
		glBindVertexArray(this->generic_vao);
		Asset::Texture_ID cache_texture_id = Asset::Texture_ID::none;
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

		// Fetch all font characters and calculate the total width.
		for (const Text &text : state.text) {
			float total_width = 0;
			Array<const Font_Face_Character *, 128> characters;
			{
				int i = 0;
				char c = text.text[i];
				while (c != '\0') {
					const Font_Face_Character *character = &this->font_face.characters[(size_t)c];
					characters.push(character);
					total_width += (character->advance_x >> 6) * text.scale;
					c = text.text[++i];
				}
			}

			// Modify the x starting position so we are the origin point of all text is at the center.
			float x = text.position.x - total_width / 2;
			float y = text.position.y;

			glUseProgram(this->text_shader_program.id);
			glBindVertexArray(this->text_vao);
			for (const Font_Face_Character *character : characters) {
				glBindTexture(GL_TEXTURE_2D, character->texture_id);

				glUniform4fv(this->text_shader_program.uniform_location.text_colour, 1, &text.colour[0]);

				const float y_offset = this->font_face.height / 2 * text.scale;
				const glm::vec3 position = glm::vec3(
					x + character->left * text.scale, 
					y - y_offset - (character->height - character->top) * text.scale, 
					0.0f
				);
				glm::mat4 transform = glm::translate(identity, position);

				const glm::vec3 size = glm::vec3(character->width * text.scale, character->height * text.scale, 1.0f);
				transform = glm::scale(transform, size);
				glUniformMatrix4fv(this->text_shader_program.uniform_location.transform, 1, GL_FALSE, &transform[0][0]);

				x += (character->advance_x >> 6) * text.scale;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		glUseProgram(this->shape_shader_program.id);
		glBindVertexArray(this->generic_vao);
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
	}

	void load_texture(unsigned char *data, Asset::Texture_ID asset_id, const Asset::Texture &texture) {
		const size_t asset_index = static_cast<size_t>(asset_id);
		glBindTexture(GL_TEXTURE_2D, this->texture_indices[asset_index]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	void load_font(int height) {
		this->font_face.height = height >> 6;
	}

	void load_font_character(unsigned char character, unsigned char *bitmap, int width, int height, int left, int top, int advance_x) {
		// TODO(steven): Find out what this is actually doing.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		Font_Face_Character &font_character = this->font_face.characters[(size_t)character];
		font_character.width = width;
		font_character.height = height;
		font_character.top = top;
		font_character.left = left;
		font_character.advance_x = advance_x;
		glGenTextures(1, &font_character.texture_id);
		
		glBindTexture(GL_TEXTURE_2D, font_character.texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font_character.width, font_character.height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

	void create_text_shader(const Shader_Contents &contents) {
		// Compile shaders
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &contents.vertex, NULL);
		glCompileShader(vertex_shader);
		this->log_shader_compile_error(vertex_shader, "Text Vertex");

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &contents.fragment, NULL);
		glCompileShader(fragment_shader);
		this->log_shader_compile_error(fragment_shader, "Text Fragment");

		this->text_shader_program.id = glCreateProgram();
		glAttachShader(this->text_shader_program.id, vertex_shader);
		glAttachShader(this->text_shader_program.id, fragment_shader);
		glLinkProgram(this->text_shader_program.id);
		this->log_shader_link_error(this->text_shader_program.id, "Text");

		glUseProgram(this->text_shader_program.id);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		// Fetch uniform locations
		this->text_shader_program.uniform_location.view_projection = glGetUniformLocation(this->text_shader_program.id, "view_projection");
		this->text_shader_program.uniform_location.transform = glGetUniformLocation(this->text_shader_program.id, "transform");
		this->text_shader_program.uniform_location.text_colour = glGetUniformLocation(this->text_shader_program.id, "text_color");
	}

	void set_viewport(const Application &application) {
		if (application.window == cached_window_size) {
			return;
		} else {
			this->cached_window_size = application.window;
			this->log("%d, %d", application.window.width, application.window.height);
		}

		const float window_aspect_ratio = (float)application.window.width / application.window.height;
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