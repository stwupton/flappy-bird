#version 330 core

in vec2 texture_coordinate0;

out vec4 colour;

uniform sampler2D tex0;
uniform vec4 text_colour;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex0, texture_coordinate0).r);
	colour = text_colour * sampled;
}