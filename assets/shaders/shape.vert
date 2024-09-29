#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 view_projection;
uniform mat4 transform;

out vec3 _position;

void main() {
	gl_Position = view_projection * transform * vec4(position, 1.0);
	_position = position;
}