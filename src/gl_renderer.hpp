#pragma once

#include <stdarg.h>
#include <string>

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "application.hpp"
#include "array.hpp"
#include "assets.hpp"
#include "game_properties.hpp"
#include "game_state.hpp"
#include "platform.hpp"
#include "debug_state.hpp"

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
		GLint view_projection;
		GLint transform;
		GLint colour;
		GLint shape_type;
	} uniform_location;
};

struct Text_Shader_Program {
	GLuint id;
	struct {
		GLint view_projection;
		GLint transform;
		GLint text_colour;
	} uniform_location;
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
	Application &application;
	Size<int> cached_window_size;

public:
	GL_Renderer(Application &application, Platform &platform) : 
		application{application}, 
		platform{platform} {}

	bool init(GLDEBUGPROC debug_message_handle) {
		// Global settings
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debug_message_handle, 0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		this->setup_shaders();
		this->setup_view_projection();

		// Generate all texture indices
		glGenTextures(static_cast<GLsizei>(this->texture_indices.size()), this->texture_indices.data());
		const bool textures_loaded_successfully = this->load_all_textures();
		if (!textures_loaded_successfully) {
			return false;
		}

		const bool font_loaded_successfully = this->load_font();
		if (!font_loaded_successfully) {
			return false;
		}

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

		return true;
	}

	void render(const Game_State &state, Debug_State *debug_state) {
		this->set_viewport();

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
					total_width += (character->advance_x >> 6) * text.scale.x;
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

				const float y_offset = this->font_face.height / 2 * text.scale.y;
				const glm::vec3 position = glm::vec3(
					x + character->left * text.scale.x, 
					y - y_offset - (character->height - character->top) * text.scale.y, 
					0.0f
				);
				glm::mat4 transform = glm::translate(identity, position);

				const glm::vec3 size = glm::vec3(character->width * text.scale.x, character->height * text.scale.y, 1.0f);
				transform = glm::scale(transform, size);
				glUniformMatrix4fv(this->text_shader_program.uniform_location.transform, 1, GL_FALSE, &transform[0][0]);

				x += (character->advance_x >> 6) * text.scale.x;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		if (debug_state != nullptr) {
			glUseProgram(this->shape_shader_program.id);
			glBindVertexArray(this->generic_vao);
			for (const Shape &debug_shape : debug_state->debug_shapes) {
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
	}

private:
	GLint get_uniform_location(GLuint program_id, const char *uniform_name, const char *shader_name) {
		const GLint location = glGetUniformLocation(program_id, uniform_name);
		if (location == -1) {
			this->log("Could not retrieve uniform location (%s) from shader (%s)", uniform_name, shader_name);
		}
		return location;
	}

	void setup_shaders() {
		this->setup_shader(&this->basic_shader_program.id, Asset::Shader_ID::basic, "Basic");
		this->basic_shader_program.uniform_location.view_projection = this->get_uniform_location(this->basic_shader_program.id, "view_projection", "Basic");
		this->basic_shader_program.uniform_location.transform = this->get_uniform_location(this->basic_shader_program.id, "transform", "Basic");

		this->setup_shader(&this->shape_shader_program.id, Asset::Shader_ID::shape, "Shape");
		this->shape_shader_program.uniform_location.view_projection = this->get_uniform_location(this->shape_shader_program.id, "view_projection", "Shape");
		this->shape_shader_program.uniform_location.transform = this->get_uniform_location(this->shape_shader_program.id, "transform", "Shape");
		this->shape_shader_program.uniform_location.colour = this->get_uniform_location(this->shape_shader_program.id, "colour", "Shape");
		this->shape_shader_program.uniform_location.shape_type = this->get_uniform_location(this->shape_shader_program.id, "shape_type", "Shape");

		this->setup_shader(&this->text_shader_program.id, Asset::Shader_ID::text, "Text");
		this->text_shader_program.uniform_location.view_projection = this->get_uniform_location(this->text_shader_program.id, "view_projection", "Text");
		this->text_shader_program.uniform_location.transform = this->get_uniform_location(this->text_shader_program.id, "transform", "Text");
		this->text_shader_program.uniform_location.text_colour = this->get_uniform_location(this->text_shader_program.id, "text_colour", "Text");
	}

	void setup_shader(GLuint *program_id, Asset::Shader_ID shader_id, const char *name) {
		const char *shader_path = Asset::get_shader(shader_id);
		const std::string file_path = this->platform.get_asset_path(shader_path);

		Platform_File *vertex_shader_file;
		const std::string vertex_shader_path = file_path + ".vert";
		this->platform.load_file(vertex_shader_path.c_str(), &vertex_shader_file);

		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, &vertex_shader_file->contents, NULL);
		glCompileShader(vertex_shader);
		this->log_shader_compile_error(vertex_shader, name, "Vertex");
		this->platform.close_file(&vertex_shader_file);

		Platform_File *fragment_shader_file;
		const std::string frag_shader_path = file_path + ".frag";
		this->platform.load_file(frag_shader_path.c_str(), &fragment_shader_file);

		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, &fragment_shader_file->contents, NULL);
		glCompileShader(fragment_shader);
		this->log_shader_compile_error(fragment_shader, name, "Fragment");
		this->platform.close_file(&fragment_shader_file);

		GLuint id = glCreateProgram();
		glAttachShader(id, vertex_shader);
		glAttachShader(id, fragment_shader);
		glLinkProgram(id);
		glUseProgram(id);
		this->log_shader_link_error(id, name);

		*program_id = id;

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
	}

	void setup_view_projection() {
		const float left = -((float)Game_Properties::view.width / 2);
		const float right = (float)Game_Properties::view.width / 2;
		const float bottom = -((float)Game_Properties::view.height / 2);
		const float top = (float)Game_Properties::view.height / 2;
		const glm::mat4 view_projection = glm::ortho(left, right, bottom, top);

		glUseProgram(this->basic_shader_program.id);
		glUniformMatrix4fv(this->basic_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);

		glUseProgram(this->shape_shader_program.id);
		glUniformMatrix4fv(this->shape_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);

		glUseProgram(this->text_shader_program.id);
		glUniformMatrix4fv(this->text_shader_program.uniform_location.view_projection, 1, GL_FALSE, &view_projection[0][0]);
	}

	bool load_all_textures() {
		std::string file_path = "";
		for (int i = 0; i < Asset::texture_data.size(); i++) {
			Asset::Texture &texture = Asset::texture_data[i];
			Asset::Texture_ID texture_id = static_cast<Asset::Texture_ID>(i);

			file_path = this->platform.get_asset_path(texture.location);

			// Don't need to do anything with `channels_in_texture` as stbi_load
			// will automatically fill in the extra channels for me.
			int channels_in_texture;
			unsigned char *data = stbi_load(
				file_path.c_str(), 
				&texture.width, 
				&texture.height, 
				&channels_in_texture, 
				4
			);

			if (data == nullptr) {
				this->log(stbi_failure_reason());
				return false;
			}

			this->load_texture(data, texture_id, texture);

			stbi_image_free(data);
		}

		return true;
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

	bool load_font() {
		std::string file_path = this->platform.get_asset_path(Asset::font_location);

		FT_Library freetype;
		if (FT_Init_FreeType(&freetype) != 0) {
			this->log("Could not init FreeType.");
			return false;
		}

		FT_Face face;
		if (FT_New_Face(freetype, file_path.c_str(), 0, &face) != 0) {
			this->log("Could not load font: %s", file_path);
			return false;
		}

		FT_Set_Pixel_Sizes(face, 0, 16);
		this->font_face.height = face->size->metrics.height >> 6;

		for (unsigned char i = 0; i < 128; i++) {
			if (FT_Load_Char(face, i, FT_LOAD_RENDER) != 0) {
				this->log("Could not load glyph: %c, in font: %s", i, file_path);
				return false;
			}

			this->load_font_character(
				i, 
				face->glyph->bitmap.buffer, 
				face->glyph->bitmap.width, 
				face->glyph->bitmap.rows, 
				face->glyph->bitmap_left,
				face->glyph->bitmap_top, 
				face->glyph->advance.x
			);
		}

		FT_Done_Face(face);
		FT_Done_FreeType(freetype);

		return true;
	}

	void load_font_character(unsigned char character, unsigned char *bitmap, int width, int height, int left, int top, int advance_x) {
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

	void set_viewport() {
		if (this->application.window == cached_window_size) {
			return;
		} else {
			this->cached_window_size = this->application.window;
			this->log("%d, %d", this->application.window.width, this->application.window.height);
		}

		const float window_aspect_ratio = (float)this->application.window.width / this->application.window.height;
		const float view_aspect_ratio = (float)Game_Properties::view.width / Game_Properties::view.height;
		
		GLsizei viewport_height;
		GLsizei viewport_width;

		const bool snap_to_height = window_aspect_ratio > view_aspect_ratio;
		if (snap_to_height) {
			viewport_height = this->application.window.height;
			viewport_width = (GLsizei)(viewport_height * view_aspect_ratio);
		} else {
			viewport_width = this->application.window.width;
			viewport_height = (GLsizei)(viewport_width / view_aspect_ratio);
		}

		const GLsizei viewport_x = this->application.window.width / 2 - viewport_width / 2;
		const GLsizei viewport_y = this->application.window.height / 2 - viewport_height / 2;

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

	void log_shader_compile_error(GLuint shader_id, const char *name, const char *shader_type) const {
		GLint success;
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE) {
			GLint message_length;
			glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &message_length);

			GLint message_byte_length = sizeof(char) * message_length;
			char *message = (char *)malloc(message_byte_length);
			memset(message, 0, message_byte_length);
			glGetShaderInfoLog(shader_id, message_length, NULL, message);

			this->log("Shader (%s - %s) failed to compile. Message: %s", name, shader_type, message);
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