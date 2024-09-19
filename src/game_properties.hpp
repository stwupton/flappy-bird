#pragma once

namespace Game_Properties {
	const int view_width = (int)(1080 * .2f); 
	const int view_height = (int)(1920 * .2f); 
	const float sim_time_s = 1.0f / 60;
	const float sim_time_ms = sim_time_s * 1000;
	const float gravity = 10.f;
	const float flap_force = 200.f;
	const float pipe_y_range = view_height * 0.2f;
	const float pipe_x_spacing = view_width;
	const float pipe_y_spacing = view_height * 0.2f;
	const float scroll_speed = 100.f;
};