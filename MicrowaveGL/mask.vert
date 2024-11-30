#version 330 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColour;

out vec3 chColour;

void main() {
	gl_Position = vec4(inPosition.xy, 0., 1.);
	chColour = inColour;
}