#pragma once

#include <glm/glm.hpp>

#include "assets.hpp"
#include "array.hpp"

struct Sprite {
	glm::mat4 transform = glm::mat4(1.f);
	Asset::Texture_ID texture;
};

struct Game_State {
	bool play_started = false;
	float ground_scroll = 0;
	Array<Sprite, 256> sprites;
	float current_gravity = 0.0f;
	glm::vec3 bird_position = glm::vec3(0.0f);
};