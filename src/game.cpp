#include "game.hpp"

const float scroll_speed = 100.f;

void Game::update(Game_State *state, Input *input, float delta) {
	// Clear the sprite
	state->sprites = {};

	// Handle first flap
	if (!state->play_started && input->flap) {
		state->play_started = true;
	}

	_bird(state, input, delta);
	_scroll_ground(state, delta);
}

void Game::_scroll_ground(Game_State *state, float delta) {
	const Asset::Texture ground_texture = Asset::get_texture(Asset::Texture_ID::ground);

	// Scroll floor
	if (state->play_started) {
		state->ground_scroll += delta * scroll_speed;
		if (state->ground_scroll >= ground_texture.width) {
			state->ground_scroll -= ground_texture.width;
		}
	}

	// Add floors to sprites
	const glm::vec2 start = glm::vec2(
		-Game_Properties::view_width / 2 + ground_texture.width / 2, 
		-Game_Properties::view_height / 2 + ground_texture.height / 2
	);

	const int number_of_ground_sprites = Game_Properties::view_width / ground_texture.width + 2;
	for (int i = 0; i < number_of_ground_sprites; i++) {
		Sprite ground_sprite = { .texture = Asset::Texture_ID::ground };
		ground_sprite.transform = glm::translate(ground_sprite.transform, glm::vec3(
			start.x + ground_texture.width * i - state->ground_scroll, 
			start.y, 
			0.f
		));
		state->sprites.push(ground_sprite);
	}
}

void Game::_bird(Game_State *state, Input *input, float delta) {
	const Asset::Texture bird_texture = Asset::get_texture(Asset::Texture_ID::bird);
	glm::mat4 bird_transform = glm::mat4(1.0f);

	// Apply gravity
	if (state->play_started) {
		if (input->hovering && state->current_gravity < .0f) {
			state->current_gravity += Game_Properties::gravity * 0.5f;
		} else {
			state->current_gravity += Game_Properties::gravity;
		}
	}

	// Flap 
	if (input->flap) {
		// TODO(steven): Workaround for not going too high, maybe collide at the top instead
		if (state->bird_position.y < Game_Properties::view_height / 2) {
			state->current_gravity = -Game_Properties::flap_force;
		}
		input->flap = false;
	}

	if (state->play_started) {
		state->bird_position.y -= state->current_gravity * delta;
		bird_transform = glm::translate(bird_transform, state->bird_position);
	}

	// Apply rotation
	// TODO(steven): Rotates too fast
	const float rotation_degrees = -glm::clamp(state->current_gravity, -10.f, 30.f);
	const float rotation_radians = rotation_degrees * glm::pi<float>() / 180;
	bird_transform = glm::rotate(bird_transform, rotation_radians, glm::vec3(0.0f, 0.0f, 1.0f));

	Sprite bird = { .transform = bird_transform, .texture = Asset::Texture_ID::bird };
	state->sprites.push(bird);
}