#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>
#include <string>

#include "application.hpp"
#include "game.hpp"
#include "persistent_game_state.hpp"
#include "game_state.hpp"
#include "gl_renderer.hpp"
#include "input.hpp"
#include "sdl_platform.hpp"
#include "debug_state.hpp"
#include "sdl_audio_player.hpp"

static GL_Renderer *renderer = nullptr;
static Application *application = nullptr;
static SDL_Platform *platform = nullptr;
static Persistent_Game_State *persistent_game_state = nullptr;
static Game_State *game_state = nullptr;
static Game_State *previous_game_state = nullptr;
static Debug_State *debug_state = nullptr;
static Input *input = nullptr;
static SDL_Audio_Player *audio_player = nullptr;

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

// Must have the main standard arguments for SDL to work.
int main(int argc, char *args[]) {
	int success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
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

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

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

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (gl_context == NULL) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	if (glewInit() != GLEW_OK) {
		SDL_Log("Could not initialise GLEW.");
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

	application = new Application();
	application->window = { 
		.width = display_bounds.w, 
		.height = display_bounds.h 
	};

	platform = new SDL_Platform();

	renderer = new GL_Renderer(*application, *platform);
	success = renderer->init(debug_message_handle); 
	if (!success) {
		return -1;
	}

	// Initialise audio
	audio_player = new SDL_Audio_Player(*platform);
	audio_player->init();

	// Initialise game states
	game_state = new Game_State();
	Game::setup(game_state);

	previous_game_state = new Game_State();

	#ifndef NDEBUG
	debug_state = new Debug_State();
	#endif

	input = new Input();

	persistent_game_state = new Persistent_Game_State();
	persistent_game_state->high_score = platform->get_high_score();

	Uint64 previous_time = SDL_GetTicks64();
	float time_accumulator = 0;

	bool should_close = false;
	while (!should_close) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EventType::SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
					#ifndef NDEBUG
					case SDLK_MINUS: {
						debug_state->sim_speed -= debug_state->sim_speed <= 1.0f ? 0.1f : 1.0f;
						debug_state->sim_speed = fmaxf(0.0f, debug_state->sim_speed);
					} break;

					case SDLK_EQUALS: {
						debug_state->sim_speed += debug_state->sim_speed < 1.0f ? 0.1f : 1.0f;
						debug_state->sim_speed = fminf(255, debug_state->sim_speed);
					} break;

					case SDLK_d: {
						debug_state->show_collision_debugger = !debug_state->show_collision_debugger;
					} break;

					case SDLK_c: {
						persistent_game_state->high_score = 0;
						platform->save_high_score(0);
					} break;
					#endif

					case SDLK_F11: {
						const bool is_fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
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

		if (debug_state != nullptr) {
			time_accumulator += (current_time - previous_time) * debug_state->sim_speed;
		} else {
			time_accumulator += current_time - previous_time;
		}

		previous_time = current_time;

		while (time_accumulator >= Game_Properties::sim_time_ms) {
			*previous_game_state = *game_state;
			time_accumulator -= Game_Properties::sim_time_ms;
			Game::update(
				game_state, 
				input, 
				persistent_game_state, 
				debug_state, 
				platform,
				audio_player, 
				Game_Properties::sim_time_s
			);
		}

		const float alpha = time_accumulator / Game_Properties::sim_time_ms;
		Game::populate_sprites(game_state, previous_game_state, alpha);

		renderer->render(*game_state, debug_state);
		SDL_GL_SwapWindow(window);
	}

	return 0;
}