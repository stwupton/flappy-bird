#version 330 core

uniform vec4 colour;
uniform int shape_type; // 0 = rect, 1 = circle

in vec3 _position;

out vec4 result_colour;

void main() {
	if (shape_type == 0) {
		result_colour = colour;
	} else {
		bool is_circle = length(_position) <= 0.5;
		result_colour = colour * float(is_circle);
	}
}