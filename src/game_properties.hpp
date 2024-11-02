#pragma once

#include "size.hpp"

namespace Game_Properties {
	const Size<int> view = {
		.width = (int)(1080 * .2f),
		.height = (int)(1920 * .2f) 
	};

	const struct {
		const size_t count = 5;
		const float x_min = -(float)view.width;
		const float x_max = (float)view.width;
		const float y_min = 64.0f;
		const float y_max = (float)view.height / 2;
		const float speed_scale_min = 0.5f;
		const float speed_scale_max = 1.0f;
		const float scale_min = 0.8f;
		const float scale_max = 1.5f;
		const float scroll_speed = 1.0f;
		const float scroll_modifier = .03f;
	} cloud;

	const float sim_time_s = 1.0f / 60;
	const float sim_time_ms = sim_time_s * 1000;

	const struct {
		const float gravity = 10.f;
		const float flap_force = 200.f;
		const float collision_radius = 12.0f;
	} bird;

	const struct {
		const float y_range = (float)view.height * 0.2f;
		const float x_spacing = (float)view.width;
		const float y_spacing = (float)view.height * 0.22f;
		const Size<float> collision_rect = {
			.width = 64.f,
			.height = 384.f
		};
	} pipe;

	const struct {
		const Size<float> size = { .width = (float)view.width, .height = 32.0f };
		const glm::vec3 position = glm::vec3(0.0f, (float)-view.height / 2 + 16, 0.0f);
	} floor_collision;

	const float scroll_speed = 100.f;

	const float hill_scroll_modifier = .2f;

	const struct {
		const glm::vec2 position = glm::vec2(0.0f, (float)view.height / 2 - 20.0f);
		const glm::vec4 colour = glm::vec4(7.0f / 255, 54.0f / 255, 66.0f / 255, 1.0f);
	} score;
};