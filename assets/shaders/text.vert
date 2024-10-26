#version 330 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec2 _texture_coordinate0;

out vec2 texture_coordinate0;

uniform mat4 view_projection;
uniform mat4 transform;

void main() {
	gl_Position = view_projection * transform * vec4(_position, 1.0);
	texture_coordinate0 = _texture_coordinate0;
}