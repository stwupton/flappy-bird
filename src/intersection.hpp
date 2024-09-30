#pragma once

#include <glm/glm.hpp>

#include "size.hpp"

inline bool circle_rect_intersection(
	glm::vec3 circle, 
	float radius, 
	glm::vec3 rect, 
	Size<float> rect_size
) {
	const float left = rect.x - rect_size.width / 2;
	const float right = rect.x + rect_size.width / 2;
	const float bottom = rect.y - rect_size.height / 2;
	const float top = rect.y + rect_size.height / 2;

	const glm::vec3 closest = glm::vec3(
		glm::clamp(circle.x, left, right), 
		glm::clamp(circle.y, bottom, top),
		0.0f
	);
	const glm::vec2 distance = closest - circle;
	return glm::length(distance) <= radius;
}