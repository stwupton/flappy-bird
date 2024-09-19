#include "game.hpp"

void Game::setup(Game_State *state) {
	// Setup the initial pipe positions when we start playing
	_setup_pipe(&state->pipes[0], Game_Properties::pipe_x_spacing);
	_setup_pipe(&state->pipes[1], Game_Properties::pipe_x_spacing * 2);
}

void Game::update(Game_State *state, Input *input, float delta) {
	// Clear the sprite
	state->sprites = {};

	// Handle first flap
	if (!state->play_started && input->flap) {
		state->play_started = true;
	}

	_pipe(state, delta);
	_scroll_ground(state, delta);
	_bird(state, input, delta);
}

void Game::_scroll_ground(Game_State *state, float delta) {
	const Asset::Texture ground_texture = Asset::get_texture(Asset::Texture_ID::ground);

	// Scroll floor
	if (state->play_started) {
		state->ground_scroll += delta * Game_Properties::scroll_speed;
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
		if (input->hovering && state->bird.y_velocity > .0f) {
			state->bird.y_velocity -= Game_Properties::gravity * 0.5f;
		} else {
			state->bird.y_velocity -= Game_Properties::gravity;
		}
	}

	// Flap 
	if (input->flap) {
		// TODO(steven): Workaround for not going too high, maybe collide at the top instead
		if (state->bird.position.y < Game_Properties::view_height / 2) {
			state->bird.y_velocity = Game_Properties::flap_force;
		}
		input->flap_handled();
	}

	if (state->play_started) {
		state->bird.position.y += state->bird.y_velocity * delta;
		bird_transform = glm::translate(bird_transform, state->bird.position);
	}

	// Apply rotation
	// TODO(steven): Rotates too fast
	const float rotation_degrees = glm::clamp(state->bird.y_velocity, -30.f, 10.f);
	const float rotation_radians = rotation_degrees * glm::pi<float>() / 180;
	bird_transform = glm::rotate(bird_transform, rotation_radians, glm::vec3(0.0f, 0.0f, 1.0f));

	Sprite bird = { .transform = bird_transform, .texture = Asset::Texture_ID::bird };
	state->sprites.push(bird);
}

void Game::_pipe(Game_State *state, float delta) {
	const Asset::Texture pipe_texture = Asset::get_texture(Asset::Texture_ID::pipe);

	for (Pipe &pipe : state->pipes) {
		// Update x position
		if (state->play_started) {
			pipe.position.x -= Game_Properties::scroll_speed * delta;
		}

		// Reset pipe if it goes off screen
		const float right_of_pipe = pipe.position.x + pipe_texture.width / 2;
		const float left_of_view = -Game_Properties::view_width / 2;
		const bool pipe_is_offscreen = right_of_pipe <= left_of_view;
		if (pipe_is_offscreen) {
			_setup_pipe(&pipe, pipe.position.x + Game_Properties::pipe_x_spacing * 2);
		}

		// Top pipe
		glm::mat4 top_pipe_transform = glm::mat4(1.0f);
		const float top_y_position = 
			pipe.position.y + 
			pipe_texture.height / 2 + 
			Game_Properties::pipe_y_spacing / 2;
		top_pipe_transform = glm::translate(
			top_pipe_transform, 
			glm::vec3(pipe.position.x, top_y_position, 0.0f)
		);
		Sprite top_pipe = { .transform = top_pipe_transform, .texture = Asset::Texture_ID::pipe };
		state->sprites.push(top_pipe);

		// Bottom pipe
		glm::mat4 bottom_pipe_transform = glm::mat4(1.0f);
		const float bottom_y_position = 
			pipe.position.y - 
			pipe_texture.height / 2 - 
			Game_Properties::pipe_y_spacing / 2;
		bottom_pipe_transform = glm::translate(
			bottom_pipe_transform, 
			glm::vec3(pipe.position.x, bottom_y_position, 0.0f)
		);
		bottom_pipe_transform = glm::scale(bottom_pipe_transform, glm::vec3(1.0f, -1.0f, 1.0f));
		Sprite bottom_pipe = { .transform = bottom_pipe_transform, .texture = Asset::Texture_ID::pipe };
		state->sprites.push(bottom_pipe);
	}
}

void Game::_setup_pipe(Pipe *pipe, float x) {
	const float y = glm::linearRand(Game_Properties::pipe_y_range, -Game_Properties::pipe_y_range);
	pipe->position = glm::vec3(x, y, 0.0f);
}