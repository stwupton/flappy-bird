#pragma once

#include <glm/glm.hpp>

#include "assets.hpp"
#include "array.hpp"
#include "size.hpp"

struct Sprite {
	glm::mat4 transform = glm::mat4(1.f);
	Asset::Texture_ID texture;
};

struct Bird {
	float y_velocity;
	glm::vec3 position = glm::vec3(0.0f);
	bool is_colliding = false;
};

struct Pipe {
	float y;
};

struct Pipe_Pair {
	float x;
	Pipe top, bottom;
};

enum class Shape_Type {
	rectangle,
	circle
};

struct Shape {
	glm::mat4 transform = glm::mat4(1.0f);
	Shape_Type type;
	glm::vec4 colour = glm::vec4(0.0f);
	union {
		struct { float radius; } circle;
		Size<float> rectangle = {};
	};
};

struct Game_State {
	bool play_started = false;
	float ground_scroll = 0;
	Array<Sprite, 256> sprites;
	Bird bird;
	Pipe_Pair pipe_pairs[2] = {};

	// Debug
	// TODO(steven): Move elsewhere?
	bool show_collision_debugger = false;
	Array<Shape, 16> debug_shapes;
};