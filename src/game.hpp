#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "game_properties.hpp"
#include "game_state.hpp"
#include "input.hpp"

namespace Game {
	void setup(Game_State *);
	void update(Game_State *, Input *, float delta);
	void _bird(Game_State *, Input *, float delta);
	void _debug_collision_shapes(Game_State *);
	void _detect_collisions(Game_State *);
	void _pipe(Game_State *, float delta);
	void _scroll_ground(Game_State *, float delta);
	void _setup_pipe(Pipe_Pair *, float x);
};