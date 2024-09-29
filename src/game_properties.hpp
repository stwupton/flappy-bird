#pragma once

namespace Game_Properties {
	const int view_width = (int)(1080 * .2f); 
	const int view_height = (int)(1920 * .2f); 
	const float sim_time_s = 1.0f / 60;
	const float sim_time_ms = sim_time_s * 1000;
	const struct {
		const float gravity = 10.f;
		const float flap_force = 200.f;
		const float collision_radius = 12.0f;
	} bird;
	const struct {
		const float y_range = view_height * 0.2f;
		const float x_spacing = view_width;
		const float y_spacing = view_height * 0.2f;
		const struct {
			const float width = 64.f;
			const float height = 384.f;
		} collision_rect;
	} pipe;
	const float scroll_speed = 100.f;
};