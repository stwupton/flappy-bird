#pragma once

#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "game_properties.hpp"
#include "game_state.hpp"
#include "persistent_game_state.hpp"
#include "platform.hpp"
#include "input.hpp"
#include "intersection.hpp"
#include "debug_state.hpp"
#include "audio_player.hpp"

struct Game {
	static void setup(Game_State *state) {
		// Setup the initial pipe positions when we start playing
		setup_pipe(&state->pipe_pairs[0], Game_Properties::pipe.x_spacing);
		setup_pipe(&state->pipe_pairs[1], Game_Properties::pipe.x_spacing * 2);

		// Setup initial clouds
		for (size_t i = 0; i < state->clouds.size(); i++) {
			Cloud &cloud = state->clouds[i];
			cloud.type = rand() % 2 == 0 ? Cloud::Type::one : Cloud::Type::two;
			cloud.speed_scale = rand_range(
				Game_Properties::cloud.speed_scale_min, 
				Game_Properties::cloud.speed_scale_max
			),
			cloud.position = glm::vec2(
				rand_range(Game_Properties::cloud.x_min, Game_Properties::cloud.x_max),
				rand_range(Game_Properties::cloud.y_min, Game_Properties::cloud.y_max)
			);
			cloud.scale = get_random_cloud_scale(i, state->clouds.size());
		}

		// Setup hills
		{
			const Asset::Texture hill_texture = Asset::get_texture(Asset::Texture_ID::hills);
			state->hills[1].position.x = hill_texture.width;
		}

		// Setup grounds
		{
			const Asset::Texture ground_texture = Asset::get_texture(Asset::Texture_ID::ground);
			float x = -Game_Properties::view.width / 2 + ground_texture.width / 2;
			const float y = -Game_Properties::view.height / 2 + ground_texture.height / 2;
			for (Ground &ground : state->grounds) {
				ground.position.x = x;
				ground.position.y = y;

				x += ground_texture.width;
			}
		}
	}

	static void update(
		Game_State *state, 
		Input *input, 
		Persistent_Game_State *persistent_state, 
		Debug_State *debug_state, 
		Platform *platform,
		Audio_Player *audio_player,
		float delta
	) {
		state->text = {};

		handle_game_reset(state, persistent_state, platform, input);

		// Handle first flap
		if (!state->play_started && input->flap) {
			state->play_started = true;
		}

		clouds(state, delta);
		hills(state, delta);
		pipe(state, delta);
		ground(state, delta);
		bird(state, input, audio_player, delta);
		score(state, persistent_state, audio_player);
		detect_collisions(state, audio_player);

		if (debug_state != nullptr) {
			debug_collision_shapes(*state, debug_state);
		}
	}

	static void populate_sprites(Game_State *state, Game_State *previous_state, float alpha) {
		// Clear the sprites
		state->sprites = {};

		// Sky
		{
			Sprite sky = { .texture = Asset::Texture_ID::sky };
			state->sprites.push(sky);
		}

		// Clouds
		for (size_t i = 0; i < state->clouds.size(); i++) {
			const Cloud &cloud = state->clouds[i];
			const Cloud &previous_cloud = previous_state->clouds[i];

			Asset::Texture_ID cloud_texture_id;
			switch (cloud.type) {
				case Cloud::Type::one: {
					cloud_texture_id = Asset::Texture_ID::cloud1;
				} break;
				case Cloud::Type::two: {
					cloud_texture_id = Asset::Texture_ID::cloud2;
				} break;
			}

			const Entity cloud_entity = cloud.lerp(previous_cloud, alpha);
			Sprite cloud_sprite = { .texture = cloud_texture_id, .transform = cloud_entity.get_transform() };
			state->sprites.push(cloud_sprite);
		}

		// Hills
		for (size_t i = 0; i < state->hills.size(); i++) {
			const Hill &hill = state->hills[i];
			const Hill &previous_hill = previous_state->hills[i];

			const Entity hill_entity = hill.lerp(previous_hill, alpha);
			Sprite hill_sprite = { .texture = Asset::Texture_ID::hills, .transform = hill_entity.get_transform() };
			state->sprites.push(hill_sprite);
		}

		// Pipes
		for (size_t pair_i = 0; pair_i < state->pipe_pairs.size(); pair_i++) {
			const Pipe_Pair &pair = state->pipe_pairs[pair_i];
			const Pipe_Pair &previous_pair = previous_state->pipe_pairs[pair_i];

			const Pipe pipes[2] = { pair.top, pair.bottom };
			const Pipe previous_pipes[2] = { previous_pair.top, previous_pair.bottom };
			for (size_t pipe_i = 0; pipe_i < 2; pipe_i++) {
				const Pipe pipe = pipes[pipe_i];
				const Pipe previous_pipe = previous_pipes[pipe_i];

				const Entity pipe_entity = pipe.lerp(previous_pipe, alpha);
				Sprite pipe_sprite = { .texture = Asset::Texture_ID::pipe, .transform = pipe_entity.get_transform() };
				state->sprites.push(pipe_sprite);
			}
		}

		// Ground
		for (size_t i = 0; i < state->grounds.size(); i++) {
			const Ground &ground = state->grounds[i];
			const Ground &previous_ground = previous_state->grounds[i];

			const Entity ground_entity = ground.lerp(previous_ground, alpha);
			Sprite ground_sprite = { .texture = Asset::Texture_ID::ground, .transform = ground_entity.get_transform() };
			state->sprites.push(ground_sprite);
		} 

		// Bird
		{
			const Entity bird_entity = state->bird.lerp(previous_state->bird, alpha);
			Sprite bird_sprite = { .texture = Asset::Texture_ID::bird, .transform = bird_entity.get_transform() };
			state->sprites.push(bird_sprite);
		}
	}

private:
	static float rand_range(float min, float max) {
		return (float)rand() / RAND_MAX * (max + min * -1) + min;
	}

	static glm::vec2 get_random_cloud_scale(size_t index, size_t length) {
		const float scale_range = Game_Properties::cloud.scale_max + Game_Properties::cloud.scale_min * -1;
		const float scale_section = scale_range / length;
		const float min = scale_section * index + Game_Properties::cloud.scale_min;
		const float max = min + scale_section;
		const float scale_result = rand_range(min, max);
		return glm::vec2(scale_result);
	}

	static void clouds(Game_State *state, float delta) {
		for (size_t i = 0; i < state->clouds.size(); i++) {
			Cloud &cloud = state->clouds[i];

			cloud.position.x -= Game_Properties::cloud.scroll_speed * cloud.speed_scale * delta;
			if (is_playing(*state)) { 
				cloud.position.x -= Game_Properties::scroll_speed * Game_Properties::cloud.scroll_modifier * delta;
			}

			// Recycle cloud 
			if (cloud.position.x <= Game_Properties::cloud.x_min) {
				cloud.version++;
				cloud.type = rand() % 2 == 0 ? Cloud::Type::one : Cloud::Type::two;
				cloud.speed_scale = rand_range(
					Game_Properties::cloud.speed_scale_min, 
					Game_Properties::cloud.speed_scale_max
				),
				cloud.position = glm::vec2(
					Game_Properties::cloud.x_max,
					rand_range(Game_Properties::cloud.y_min, Game_Properties::cloud.y_max)
				);
				cloud.scale = get_random_cloud_scale(i, state->clouds.size());
			}
		}
	}

	static void debug_collision_shapes(const Game_State &state, Debug_State *debug_state) {
		debug_state->debug_shapes = {};

		if (!debug_state->show_collision_debugger) {
			return;
		}

		const glm::vec4 red = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);
		const glm::vec4 green = glm::vec4(0.0f, 1.0f, 0.0f, 0.5f);
		const glm::vec4 blue = glm::vec4(0.0f, 0.0f, 1.0f, 0.5f);

		Shape bird_collision_shape = {};
		bird_collision_shape.transform = glm::translate(bird_collision_shape.transform, glm::vec3(state.bird.position, 0.0f));
		bird_collision_shape.colour = state.bird.is_colliding ? red : green;
		bird_collision_shape.type = Shape_Type::circle;
		bird_collision_shape.circle = { .radius = Game_Properties::bird.collision_radius };
		debug_state->debug_shapes.push(bird_collision_shape);

		for (const Pipe_Pair &pair : state.pipe_pairs) {
			Shape top_collision_shape = {};
			top_collision_shape.transform = glm::translate(
				top_collision_shape.transform, 
				glm::vec3(pair.top.position.x, pair.top.position.y, 0.0f)
			);
			top_collision_shape.colour = blue;
			top_collision_shape.type = Shape_Type::rectangle;
			top_collision_shape.rectangle = { 
				.width = Game_Properties::pipe.collision_rect.width, 
				.height = Game_Properties::pipe.collision_rect.height, 
			};
			debug_state->debug_shapes.push(top_collision_shape);

			Shape bottom_collision_shape = {};
			bottom_collision_shape.transform = glm::translate(
				bottom_collision_shape.transform, 
				glm::vec3(pair.bottom.position.x, pair.bottom.position.y, 0.0f)
			);
			bottom_collision_shape.colour = blue;
			bottom_collision_shape.type = Shape_Type::rectangle;
			bottom_collision_shape.rectangle = { 
				.width = Game_Properties::pipe.collision_rect.width, 
				.height = Game_Properties::pipe.collision_rect.height, 
			};
			debug_state->debug_shapes.push(bottom_collision_shape);
		}

		Shape floor_collision_shape = {};
		floor_collision_shape.transform = glm::translate(glm::mat4(1.0f), Game_Properties::floor_collision.position);
		floor_collision_shape.type = Shape_Type::rectangle;
		floor_collision_shape.rectangle = Game_Properties::floor_collision.size;
		floor_collision_shape.colour = blue;
		debug_state->debug_shapes.push(floor_collision_shape);
	}

	static void detect_collisions(Game_State *state, Audio_Player *audio_player) {
		if (state->bird.is_colliding) {
			return;
		}

		bool is_colliding = false;

		for (const Pipe_Pair &pair : state->pipe_pairs) {
			for (const Pipe &pipe : { pair.top, pair.bottom }) {
				const bool is_intersecting = circle_rect_intersection(
					state->bird.position, 
					Game_Properties::bird.collision_radius, 
					pipe.position, 
					Game_Properties::pipe.collision_rect
				);

				if (is_intersecting) {
					is_colliding = true;
				}
			}
		}

		is_colliding = is_colliding || circle_rect_intersection(
			state->bird.position, 
			Game_Properties::bird.collision_radius, 
			Game_Properties::floor_collision.position, 
			Game_Properties::floor_collision.size
		);

		state->bird.is_colliding = is_colliding;
		if (is_colliding) {
			audio_player->hit();
		}
	}

	static void handle_game_reset(
		Game_State *state, 
		Persistent_Game_State *persistent_state, 
		Platform *platform, 
		Input *input
	) {
		if (!state->bird.is_colliding) {
			return;
		}

		const bool should_reset = state->bird.position.y <= -Game_Properties::view.height;
		if (should_reset) {
			if (state->score > persistent_state->high_score) {
				persistent_state->high_score = state->score;
				platform->save_high_score(persistent_state->high_score);
			}

			*state = {};
			*input = {};
			setup(state);
		}
	}

	static void ground(Game_State *state, float delta) {
		const Asset::Texture ground_texture = Asset::get_texture(Asset::Texture_ID::ground);

		if (is_playing(*state)) {
			for (Ground &ground : state->grounds) {
				ground.position.x -= Game_Properties::scroll_speed * delta;

				// Isn't actually right of ground, needed to give it some more 
				// padding to prevent artifact.
				const float right_of_ground = ground.position.x + ground_texture.width;
				const float left_of_screen = -Game_Properties::view.width / 2;
				if (right_of_ground <= left_of_screen) {
					ground.version++;
					ground.position.x += ground_texture.width * state->grounds.size();
				}
			}
		}
	}

	static void hills(Game_State *state, float delta) {
		const Asset::Texture hills_texture = Asset::get_texture(Asset::Texture_ID::hills);

		// Scroll hills
		for (Hill &hill : state->hills) {
			hill.position.y = -Game_Properties::view.height / 2 + hills_texture.height / 2;
			if (is_playing(*state)) {
				hill.position.x -= Game_Properties::scroll_speed * Game_Properties::hill_scroll_modifier * delta;
				if (hill.position.x <= -hills_texture.width) {
					hill.version++;
					hill.position.x += hills_texture.width * 2;
				}
			}
		}
	}

	static bool is_playing(const Game_State &state) {
		return state.play_started && !state.bird.is_colliding;
	}
	
	static void bird(Game_State *state, Input *input, Audio_Player *audio_player, float delta) {
		const Asset::Texture bird_texture = Asset::get_texture(Asset::Texture_ID::bird);

		// Apply gravity
		if (state->play_started) {
			if (input->hovering && state->bird.y_velocity > .0f) {
				state->bird.y_velocity -= Game_Properties::bird.gravity * Game_Properties::bird.hovering_scale;
			} else {
				state->bird.y_velocity -= Game_Properties::bird.gravity;
			}
		}

		// Flap 
		if (input->flap && !state->bird.is_colliding) {
			if (state->bird.position.y < Game_Properties::view.height / 2) {
				state->bird.y_velocity = Game_Properties::bird.flap_force;
			}
			input->flap_handled();
			audio_player->flap();
		}

		if (state->play_started) {
			state->bird.position.y += state->bird.y_velocity * delta;
		}

		// Apply rotation
		const float rotation_smooth_scalar = .2f;
		const float rotation_degrees = glm::clamp(state->bird.y_velocity * rotation_smooth_scalar, -30.f, 10.f);
		const float rotation_radians = rotation_degrees * glm::pi<float>() / 180;
		state->bird.rotation = rotation_radians;
	}

	static void pipe(Game_State *state, float delta) {
		const Asset::Texture pipe_texture = Asset::get_texture(Asset::Texture_ID::pipe);

		for (Pipe_Pair &pair : state->pipe_pairs) {
			if (is_playing(*state)) {
				// Update x position
				pair.shared_x -= Game_Properties::scroll_speed * delta;

				// Reset pipe if it goes off screen
				const float right_of_pipe = pair.shared_x + pipe_texture.width / 2;
				const float left_of_view = -(float)Game_Properties::view.width / 2;
				const bool pipe_is_offscreen = right_of_pipe <= left_of_view;
				if (pipe_is_offscreen) {
					setup_pipe(&pair, pair.shared_x + Game_Properties::pipe.x_spacing * 2);
				}
			}

			Pipe *pipes[2] = { &pair.top, &pair.bottom };
			for (Pipe *pipe : pipes) {
				pipe->position.x = pair.shared_x;
			}
		}
	}

	static void score(
		Game_State *state, 
		Persistent_Game_State *persistent_state, 
		Audio_Player *audio_player
	) {
		int score = persistent_state->high_score;

		if (state->play_started) {
			// Update score
			for (int i = 0; i < state->pipe_pairs.size(); i++) {
				if (state->pipe_pairs[i].shared_x <= 0 && state->last_scoring_pipe_index != i) {
					state->last_scoring_pipe_index = i;
					state->score++;
					audio_player->score();
				}
			}

			score = state->score;
		} else {
			Text high_score_label = {};
			high_score_label.position = Game_Properties::score_label.position;
			high_score_label.colour = Game_Properties::score_label.colour;
			high_score_label.scale = glm::vec2(Game_Properties::score_label.scale);
			sprintf(high_score_label.text, "%s", "HIGH SCORE");
			state->text.push(high_score_label);
		}

		Text score_text = {};
		score_text.position = Game_Properties::score.position;
		score_text.colour = Game_Properties::score.colour;
		sprintf(score_text.text, "%d", score);
		state->text.push(score_text);
	}

	static void setup_pipe(Pipe_Pair *pair, float x) {
		pair->top.version++;
		pair->bottom.version++;

		const Asset::Texture pipe_texture = Asset::get_texture(Asset::Texture_ID::pipe);
		const float y = (
			(float)rand() / RAND_MAX * 
			Game_Properties::pipe.y_range * 2 - 
			Game_Properties::pipe.y_range
		);
		pair->shared_x = x;
		pair->top.position.y = y + pipe_texture.height / 2 + Game_Properties::pipe.y_spacing / 2;
		pair->bottom.position.y = y - pipe_texture.height / 2 - Game_Properties::pipe.y_spacing / 2;

		// Flip bottom pipe
		pair->bottom.scale.y = -1.0f;
	}
};