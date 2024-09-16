#version 330 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec2 _texCoord0;

out vec2 texCoord0;

uniform vec2 texture_size;
uniform mat4 view_projection;
uniform mat4 transform;

void main() {
	gl_Position = view_projection * transform * vec4(
		_position.x * texture_size.x, 
		_position.y * texture_size.y, 
		_position.z, 
		1.0
	);
	texCoord0 = _texCoord0;
}