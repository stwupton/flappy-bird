#version 330 core

in vec2 texture_coordinate0;

out vec4 color;

uniform sampler2D tex0;
uniform vec4 text_color;

void main() {
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex0, texture_coordinate0).r);
	color = text_color * sampled;
}