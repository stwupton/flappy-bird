#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "array.hpp"
#include "size.hpp"

enum class Shape_Type {
	rectangle,
	circle
};

struct Shape {
	glm::mat4 transform = glm::identity<glm::mat4>();
	Shape_Type type;
	glm::vec4 colour = glm::vec4(0.0f);
	union {
		struct { float radius; } circle;
		Size<float> rectangle = {};
	};
};

struct Debug_State {
	bool show_collision_debugger = false;
	Array<Shape, 16> debug_shapes;
};