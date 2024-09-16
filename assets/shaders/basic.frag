#version 330 core

in vec2 texCoord0;

out vec4 color;

uniform sampler2D tex0;

void main() {
	color = texture(tex0, texCoord0);
}