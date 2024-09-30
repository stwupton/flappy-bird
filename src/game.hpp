#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "game_properties.hpp"
#include "game_state.hpp"
#include "input.hpp"
#include "intersection.hpp"

struct Game {
	static void setup(Game_State *state) {
		// Setup the initial pipe positions when we start playing
		_setup_pipe(&state->pipe_pairs[0], Game_Properties::pipe.x_spacing);
		_setup_pipe(&state->pipe_pairs[1], Game_Properties::pipe.x_spacing * 2);
	}

	static void update(Game_State *state, Input *input, float delta) {
		// Clear the sprite
		state->sprites = {};

		// Handle first flap
		if (!state->play_started && input->flap) {
			state->play_started = true;
		}

		_pipe(state, delta);
		_scroll_ground(state, delta);
		_bird(state, input, delta);

		_detect_collisions(state);

		state->debug_shapes = {};
		_debug_collision_shapes(state);
	}

	static void _debug_collision_shapes(Game_State *state) {
		if (!state->show_collision_debugger) {
			return;
		}

		const glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
		const glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 0.5f);
		const glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 0.5f);

		Shape bird_collision_shape = {};
		bird_collision_shape.transform = glm::translate(bird_collision_shape.transform, state->bird.position);
		bird_collision_shape.colour = state->bird.is_colliding ? red : green;
		bird_collision_shape.type = Shape_Type::circle;
		bird_collision_shape.circle = { .radius = Game_Properties::bird.collision_radius };
		state->debug_shapes.push(bird_collision_shape);

		for (const Pipe_Pair &pair : state->pipe_pairs) {
			Shape top_collision_shape = {};
			top_collision_shape.transform = glm::translate(top_collision_shape.transform, glm::vec3(pair.x, pair.top.y, 0.0f));
			top_collision_shape.colour = blue;
			top_collision_shape.type = Shape_Type::rectangle;
			top_collision_shape.rectangle = { 
				.width = Game_Properties::pipe.collision_rect.width, 
				.height = Game_Properties::pipe.collision_rect.height, 
			};
			state->debug_shapes.push(top_collision_shape);

			Shape bottom_collision_shape = {};
			bottom_collision_shape.transform = glm::translate(bottom_collision_shape.transform, glm::vec3(pair.x, pair.bottom.y, 0.0f));
			bottom_collision_shape.colour = blue;
			bottom_collision_shape.type = Shape_Type::rectangle;
			bottom_collision_shape.rectangle = { 
				.width = Game_Properties::pipe.collision_rect.width, 
				.height = Game_Properties::pipe.collision_rect.height, 
			};
			state->debug_shapes.push(bottom_collision_shape);
		}

		Shape floor_collision_shape = {};
		floor_collision_shape.transform = glm::translate(glm::mat4(1.0f), Game_Properties::floor_collision.position);
		floor_collision_shape.type = Shape_Type::rectangle;
		floor_collision_shape.rectangle = Game_Properties::floor_collision.size;
		floor_collision_shape.colour = blue;
		state->debug_shapes.push(floor_collision_shape);
	}

	static void _detect_collisions(Game_State *state) {
		state->bird.is_colliding = false;

		for (const Pipe_Pair &pair : state->pipe_pairs) {
			for (const Pipe pipe : { pair.top, pair.bottom }) {
				const bool is_intersecting = circle_rect_intersection(
					state->bird.position, 
					Game_Properties::bird.collision_radius, 
					glm::vec3(pair.x, pipe.y, 0.0f), 
					Game_Properties::pipe.collision_rect
				);

				if (is_intersecting) {
					state->bird.is_colliding = true;
					return;
				}
			}
		}

		state->bird.is_colliding = circle_rect_intersection(
			state->bird.position, 
			Game_Properties::bird.collision_radius, 
			Game_Properties::floor_collision.position, 
			Game_Properties::floor_collision.size
		);
	}

	static void _scroll_ground(Game_State *state, float delta) {
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
			-Game_Properties::view.width / 2 + ground_texture.width / 2, 
			-Game_Properties::view.height / 2 + ground_texture.height / 2
		);

		const int number_of_ground_sprites = Game_Properties::view.width / ground_texture.width + 2;
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

	static void _bird(Game_State *state, Input *input, float delta) {
		const Asset::Texture bird_texture = Asset::get_texture(Asset::Texture_ID::bird);
		glm::mat4 bird_transform = glm::mat4(1.0f);

		// Apply gravity
		if (state->play_started) {
			if (input->hovering && state->bird.y_velocity > .0f) {
				state->bird.y_velocity -= Game_Properties::bird.gravity * 0.5f;
			} else {
				state->bird.y_velocity -= Game_Properties::bird.gravity;
			}
		}

		// Flap 
		if (input->flap) {
			// TODO(steven): Workaround for not going too high, maybe collide at the top instead
			if (state->bird.position.y < Game_Properties::view.height / 2) {
				state->bird.y_velocity = Game_Properties::bird.flap_force;
			}
			input->flap_handled();
		}

		if (state->play_started) {
			state->bird.position.y += state->bird.y_velocity * delta;
			bird_transform = glm::translate(bird_transform, state->bird.position);
		}

		// Apply rotation
		const float rotation_smooth_scalar = .2f;
		const float rotation_degrees = glm::clamp(state->bird.y_velocity * rotation_smooth_scalar, -30.f, 10.f);
		const float rotation_radians = rotation_degrees * glm::pi<float>() / 180;
		bird_transform = glm::rotate(bird_transform, rotation_radians, glm::vec3(0.0f, 0.0f, 1.0f));

		Sprite bird = { .transform = bird_transform, .texture = Asset::Texture_ID::bird };
		state->sprites.push(bird);
	}

	static void _pipe(Game_State *state, float delta) {
		const Asset::Texture pipe_texture = Asset::get_texture(Asset::Texture_ID::pipe);

		for (Pipe_Pair &pair : state->pipe_pairs) {
			// Update x position
			if (state->play_started) {
				pair.x -= Game_Properties::scroll_speed * delta;
			}

			// Reset pipe if it goes off screen
			const float right_of_pipe = pair.x + pipe_texture.width / 2;
			const float left_of_view = -(float)Game_Properties::view.width / 2;
			const bool pipe_is_offscreen = right_of_pipe <= left_of_view;
			if (pipe_is_offscreen) {
				_setup_pipe(&pair, pair.x + Game_Properties::pipe.x_spacing * 2);
			}

			// Top pipe
			Sprite top_pipe = { .texture = Asset::Texture_ID::pipe };
			top_pipe.transform = glm::translate(
				top_pipe.transform, 
				glm::vec3(pair.x, pair.top.y, 0.0f)
			);
			state->sprites.push(top_pipe);

			// Bottom pipe
			Sprite bottom_pipe = { .texture = Asset::Texture_ID::pipe };
			bottom_pipe.transform = glm::translate(
				bottom_pipe.transform, 
				glm::vec3(pair.x, pair.bottom.y, 0.0f)
			);
			bottom_pipe.transform = glm::scale(bottom_pipe.transform, glm::vec3(1.0f, -1.0f, 1.0f));
			state->sprites.push(bottom_pipe);
		}
	}

	static void _setup_pipe(Pipe_Pair *pair, float x) {
		const Asset::Texture pipe_texture = Asset::get_texture(Asset::Texture_ID::pipe);
		const float y = (
			(float)rand() / RAND_MAX * 
			Game_Properties::pipe.y_range * 2 - 
			Game_Properties::pipe.y_range
		);
		pair->x = x;
		pair->top.y = y + pipe_texture.height / 2 + Game_Properties::pipe.y_spacing / 2;
		pair->bottom.y = y - pipe_texture.height / 2 - Game_Properties::pipe.y_spacing / 2;
	}
};