#pragma once

#include <glm/glm.hpp>

#include "assets.hpp"
#include "array.hpp"
#include "game_properties.hpp"
#include "size.hpp"

struct Sprite {
	Asset::Texture_ID texture;
	glm::mat4 transform = glm::mat4(1.f);
};

struct Entity {
	// Version ID to keep track if it's the same entity as before. It is commonly
	// changed when recycling the entity when it leaves the view.
	int version = 0;

	glm::vec2 position = glm::vec2(0.0f);
	float rotation = 0.0f;
	glm::vec2 scale = glm::vec2(1.0f);

	// Used to interpolate entities from the previous state only if they are the
	// same version.
	Entity lerp(const Entity &from, float alpha) const {
		if (this->version == from.version) {
			return Entity {
				.position = glm::mix(from.position, this->position, alpha),
				.rotation = glm::mix(from.rotation, this->rotation, alpha),
				.scale = glm::mix(from.scale, this->scale, alpha),
			};
		}

		return *this;
	}

	glm::mat4 get_transform() const {
		glm::mat4 transform = glm::identity<glm::mat4>();
		transform = glm::translate(transform, glm::vec3(this->position.x, this->position.y, 0.0f));
		transform = glm::rotate(transform, this->rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, glm::vec3(this->scale.x, this->scale.y, 1.0f));
		return transform;
	}
};

struct Bird : Entity {
	float y_velocity;
	bool is_colliding = false;
};

struct Pipe : Entity {};

struct Pipe_Pair {
	float shared_x = 0.0f;
	Pipe top, bottom;
};

struct Text : Entity {
	glm::vec4 colour;
	char text[128];
};

struct Cloud : Entity {
	enum class Type {
		one, two
	};

	float speed_scale = 1.0f;
	Type type;
};

struct Hill : Entity {};

struct Ground : Entity {};

struct Game_State {
	bool play_started = false;
	int score = 0;
	int last_scoring_pipe_index = -1;

	Bird bird;
	std::array<Cloud, 5> clouds = {};
	std::array<Hill, 2> hills;
	std::array<Pipe_Pair, 2> pipe_pairs = {};
	std::array<Ground, 9> grounds;

	Array<Sprite, 256> sprites;
	Array<Text, 2> text;
};