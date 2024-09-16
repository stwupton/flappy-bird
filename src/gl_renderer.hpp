#pragma once

#include <array>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "array.hpp"
#include "application.hpp"
#include "assets.hpp"
#include "game_properties.hpp"
#include "game_state.hpp"

#define DEBUG_MESSAGE_CALLBACK GLDEBUGPROC

struct GL_Renderer {
	std::array<GLuint, static_cast<size_t>(Asset::Texture_ID::_length)> texture_indices = {};
	GLuint shader_program;

	void init(DEBUG_MESSAGE_CALLBACK, const char *vertex_contents, const char *fragment_contents);
	void render(const Application &, const Game_State &) const;
	void load(unsigned char * data, Asset::Texture_ID, const Asset::Texture &);
};