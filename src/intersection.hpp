#pragma once

#include <glm/glm.hpp>

#include "size.hpp"

inline bool circle_rect_intersection(
	glm::vec2 circle, 
	float radius, 
	glm::vec2 rect, 
	Size<float> rect_size
) {
	const float left = rect.x - rect_size.width / 2;
	const float right = rect.x + rect_size.width / 2;
	const float bottom = rect.y - rect_size.height / 2;
	const float top = rect.y + rect_size.height / 2;

	const glm::vec2 closest = glm::vec2(
		glm::clamp(circle.x, left, right), 
		glm::clamp(circle.y, bottom, top)
	);
	const glm::vec2 distance = closest - circle;
	return glm::length(distance) <= radius;
}