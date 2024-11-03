#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "application.hpp"
#include "game.hpp"
#include "persistent_game_state.hpp"
#include "game_state.hpp"
#include "gl_renderer.hpp"
#include "input.hpp"
#include "sdl_platform.hpp"

static GL_Renderer *renderer = nullptr;
static Application *application = nullptr;
static SDL_Platform *platform = nullptr;
static Persistent_Game_State *persistent_game_state = nullptr;
static Game_State *game_state = nullptr;
static Input *input = nullptr;

void debug_message_handle(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *user_param
) {
	if (type == GL_DEBUG_TYPE_ERROR) {
		platform->log_error("GL Error: Severity: %i, Message: %s", severity, message);
	} else {
		platform->log_info("GL Debug Message: %s", message);
	}
}

Shader_Contents get_shader_contents(const char *name) {
	std::string file_path = "";
	std::string executable_location = SDL_GetBasePath();
	SDL_RWops *file;
	Sint64 file_size = 0;

	// Vertex Shader
	file_path = executable_location + "assets/shaders/" + name + ".vert";
	file = SDL_RWFromFile(file_path.c_str(), "r");
	file_size = SDL_RWsize(file);
	char *vertex_contents = (char *)malloc(sizeof(char) * (file_size + 1));
	memset(vertex_contents, 0, sizeof(char) * (file_size + 1));
	SDL_RWread(file, vertex_contents, sizeof(char), file_size);
	SDL_RWclose(file);

	// Fragment Shader
	file_path = executable_location + "assets/shaders/" + name + ".frag";
	file = SDL_RWFromFile(file_path.c_str(), "r");
	file_size = SDL_RWsize(file);
	char *fragment_contents = (char *)malloc(sizeof(char) * (file_size + 1));
	memset(fragment_contents, 0, sizeof(char) * (file_size + 1));
	SDL_RWread(file, fragment_contents, sizeof(char), file_size);
	SDL_RWclose(file);

	return { .vertex = vertex_contents, .fragment = fragment_contents };
}

void free_shader_contents(Shader_Contents *contents) {
	free((void *)contents->vertex);
	free((void *)contents->fragment);
}

// Must have the main standard arguments for SDL to work.
int main(int argc, char *args[]) {
	int success = SDL_Init(SDL_INIT_VIDEO);
	if (success != 0) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	SDL_Rect display_bounds;
	success = SDL_GetDisplayBounds(0, &display_bounds);
	if (success != 0) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	const int window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED;
	SDL_Window *window = SDL_CreateWindow(
		"Flappy Bird", 
		display_bounds.x, 
		display_bounds.y, 
		display_bounds.w, 
		display_bounds.h, 
		window_flags
	);
	if (window == nullptr) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (gl_context == NULL) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	success = SDL_GL_SetSwapInterval(-1);
	
	const bool vsync_not_supported = success == -1;
	if (vsync_not_supported) {
		success = SDL_GL_SetSwapInterval(1);
	}

	if (success != 0) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	if (glewInit() != GLEW_OK) {
		SDL_Log("Could not initialise GLEW.");
		return -1;
	}

	application = new Application();
	application->window = { 
		.width = display_bounds.w, 
		.height = display_bounds.h 
	};

	platform = new SDL_Platform();

	// TODO(steven): Clean up loading

	// Get shader contents
	Shader_Contents basic_shader_contents = get_shader_contents("basic");
	Shader_Contents shape_shader_contents = get_shader_contents("shape");
	Shader_Contents text_shader_contents = get_shader_contents("text");

	renderer = new GL_Renderer(*platform);
	renderer->init(
		*application, 
		debug_message_handle, 
		basic_shader_contents, 
		shape_shader_contents, 
		text_shader_contents
	); 

	// Delete shader file contents
	free_shader_contents(&basic_shader_contents);
	free_shader_contents(&shape_shader_contents);

	// Load all textures
	std::string file_path = "";
	std::string executable_location = SDL_GetBasePath();
	for (int i = 0; i < Asset::texture_data.size(); i++) {
		Asset::Texture &texture = Asset::texture_data[i];
		Asset::Texture_ID texture_id = static_cast<Asset::Texture_ID>(i);

		file_path = executable_location + texture.location;

		// TODO(steven): Do we need this?
		int channels;
		unsigned char *data = stbi_load(
			file_path.c_str(), 
			&texture.width, 
			&texture.height, 
			&channels, 
			4
		);

		if (data == nullptr) {
			SDL_Log(stbi_failure_reason());
			return -1;
		}

		renderer->load_texture(data, texture_id, texture);

		stbi_image_free(data);
	}

	// Load font 
	{
		file_path = executable_location + "assets/fonts/PressStart2P-Regular.ttf", "rb";
		FT_Library freetype;
		if (FT_Init_FreeType(&freetype) != 0) {
			SDL_Log("Could not init FreeType.");
			return -1;
		}

		FT_Face face;
		if (FT_New_Face(freetype, file_path.c_str(), 0, &face) != 0) {
			SDL_Log("Could not load font: %s", file_path);
			return -1;
		}

		FT_Set_Pixel_Sizes(face, 0, 16);

		renderer->load_font(face->size->metrics.height);

		for (unsigned char i = 0; i < 128; i++) {
			if (FT_Load_Char(face, i, FT_LOAD_RENDER) != 0) {
				SDL_Log("Could not load glyph: %c, in font: %s", i, file_path);
				return -1;
			}

			renderer->load_font_character(
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
	}

	game_state = new Game_State();
	Game::setup(game_state);

	input = new Input();

	persistent_game_state = new Persistent_Game_State();
	persistent_game_state->high_score = platform->get_high_score();

	Uint64 previous_time = SDL_GetTicks64();
	float time_accumulator = 0;
	float sim_speed = 1;

	bool should_close = false;
	while (!should_close) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EventType::SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					case SDLK_MINUS: {
						sim_speed -= sim_speed <= 1.0f ? 0.1f : 1.0f;
						sim_speed = fmaxf(0.0f, sim_speed);
					} break;

					case SDLK_EQUALS: {
						sim_speed += sim_speed < 1.0f ? 0.1f : 1.0f;
						sim_speed = fminf(255, sim_speed);
					} break;

					case SDLK_d: {
						game_state->show_collision_debugger = !game_state->show_collision_debugger;
					} break;

					case SDLK_F11: {
						const bool is_fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
						if (is_fullscreen) {
							SDL_SetWindowResizable(window, SDL_TRUE);
							SDL_SetWindowFullscreen(window, 0);
						} else {
							SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
					} break;
				}
			} else if (event.type == SDL_EventType::SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == 1) {
					input->input_down();
				}
			} else if (event.type == SDL_EventType::SDL_MOUSEBUTTONUP) {
				if (event.button.button == 1) {
					input->input_up();
				}
			} else if (event.type == SDL_EventType::SDL_QUIT) {
				should_close = true;
			} else if (
				event.type == SDL_EventType::SDL_WINDOWEVENT && 
				event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
			) {
				application->window.width = event.window.data1;
				application->window.height = event.window.data2;
			}
		}

		const Uint64 current_time = SDL_GetTicks64();
		time_accumulator += (current_time - previous_time) * sim_speed;
		previous_time = current_time;

		while (time_accumulator >= Game_Properties::sim_time_ms) {
			time_accumulator -= Game_Properties::sim_time_ms;
			Game::update(game_state, input, persistent_game_state, platform, Game_Properties::sim_time_s);
		}

		renderer->render(*application, *game_state);
		SDL_GL_SwapWindow(window);
	}

	return 0;
}