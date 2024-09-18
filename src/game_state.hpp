#pragma once

#include <glm/glm.hpp>

#include "assets.hpp"
#include "array.hpp"

struct Sprite {
	glm::mat4 transform = glm::mat4(1.f);
	Asset::Texture_ID texture;
};

struct Bird {
	float y_velocity;
	glm::vec3 position = glm::vec3(0.0f);
};

struct Game_State {
	bool play_started = false;
	float ground_scroll = 0;
	Array<Sprite, 256> sprites;
	Bird bird;
};