#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "application.hpp"
#include "gl_renderer.hpp"
#include "game.hpp"
#include "game_state.hpp"
#include "input.hpp"

static GL_Renderer *renderer = nullptr;
static Application *application = nullptr;
static Game_State *game_state = nullptr;
static Input *input = nullptr;

void handle_gl_error(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *user_param
) {
	if (type == GL_DEBUG_TYPE_ERROR) {
		SDL_LogError(SDL_LOG_PRIORITY_ERROR, "GL Error - Severity: %i, Message: %s", severity, message);
	} else {
		SDL_Log("GL Debug Message - %s", message);
	}
}

int get_display_bounds(SDL_Rect *display_bounds) {
	int success;

	int display_index = -1;
	int display_score = 0;
	SDL_DisplayMode display_mode;

	const int num_video_displays = SDL_GetNumVideoDisplays();
	for (int i = 0; i < num_video_displays; i++) {
		SDL_DisplayMode this_display_mode; 

		success = SDL_GetDesktopDisplayMode(i, &this_display_mode);
		if (success != 0) {
			return success;
		}

		const float aspect_ratio = (float)this_display_mode.w / this_display_mode.h;

		// TODO(steven): Temp algorithm to select display device. Make it better.
		// TODO(steven): Change to look for a portrait screen first. Atm it's looking for landscape.
		const int this_display_score = 
			this_display_mode.w * 
			this_display_mode.h * 
			this_display_mode.refresh_rate *
			aspect_ratio;
		// const int this_display_score = 
		// 	this_display_mode.w * 
		// 	this_display_mode.h * 
		// 	this_display_mode.refresh_rate;

		if (this_display_score > display_score) {
			display_score = this_display_score;
			display_mode = this_display_mode;
			display_index = i;
		}
	}

	const char *display_name = SDL_GetDisplayName(display_index);
	SDL_Log(
		"display name: %s, format: %d, width: %d, height: %d, refresh rate: %d", 
		display_name, 
		display_mode.format,
		display_mode.w,
		display_mode.h,
		display_mode.refresh_rate
	);

	success = SDL_GetDisplayBounds(display_index, display_bounds);
	return success;
}

// Must have the main standard arguments for SDL to work.
int main(int argc, char *args[]) {
	int success = SDL_Init(SDL_INIT_EVERYTHING);
	if (success != 0) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	SDL_Rect display_bounds; 
	success = get_display_bounds(&display_bounds);
	if (success != 0) {
		SDL_Log(SDL_GetError());
		return -1;
	}

	const int window_flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL;
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

	// TODO(steven): Clean up loading

	// Get shader contents
	const size_t file_path_max = 256;
	std::string file_path = "";
	std::string executable_location = SDL_GetBasePath();
	SDL_RWops *file;
	Sint64 file_size = 0;

	// Vertex Shader
	file_path = executable_location + "assets/shaders/basic.vert";
	file = SDL_RWFromFile(file_path.c_str(), "r");
	file_size = SDL_RWsize(file);
	char *vertex_contents = (char *)malloc(sizeof(char) * (file_size + 1));
	memset(vertex_contents, 0, sizeof(char) * (file_size + 1));
	SDL_RWread(file, vertex_contents, sizeof(char), file_size);
	SDL_RWclose(file);

	// Fragment Shader
	file_path = executable_location + "assets/shaders/basic.frag";
	file = SDL_RWFromFile(file_path.c_str(), "r");
	file_size = SDL_RWsize(file);
	char *fragment_contents = (char *)malloc(sizeof(char) * (file_size + 1));
	memset(fragment_contents, 0, sizeof(char) * (file_size + 1));
	SDL_RWread(file, fragment_contents, sizeof(char), file_size);
	SDL_RWclose(file);

	renderer = new GL_Renderer();
	renderer->init(handle_gl_error, vertex_contents, fragment_contents);

	// Delete shader file contents
	free(vertex_contents);
	free(fragment_contents);

	// Load all textures
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

		renderer->load(data, texture_id, texture);

		stbi_image_free(data);
	}

	application = new Application();
	application->window = { 
		.width = display_bounds.w, 
		.height = display_bounds.h 
	};

	game_state = new Game_State();
	input = new Input();

	Uint64 previous_time = SDL_GetTicks64();
	float time_accumulator = 0;
	int sim_speed = 1;

	bool should_close = false;
	while (!should_close) {
		const Uint64 current_time = SDL_GetTicks64();
		time_accumulator += (current_time - previous_time) * sim_speed;
		previous_time = current_time;

		while (time_accumulator >= Game_Properties::sim_time_ms) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EventType::SDL_KEYDOWN) {
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE: {
							should_close = true;
						} break;
					}
				} else if (event.type == SDL_EventType::SDL_MOUSEBUTTONDOWN) {
					if (event.button.button == 1) {
						input->flap = true;
						input->hovering = true;
					}
				} else if (event.type == SDL_EventType::SDL_MOUSEBUTTONUP) {
					if (event.button.button == 1) {
						input->hovering = false;
					}
				}
			}

			time_accumulator -= Game_Properties::sim_time_ms;
			Game::update(game_state, input, Game_Properties::sim_time_s);
		}

		renderer->render(*application, *game_state);
		SDL_GL_SwapWindow(window);
	}

	return 0;
}