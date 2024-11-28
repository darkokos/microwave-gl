#version 330 core

in vec3 chColour;

out vec4 outColour;

void main() {
	outColour = vec4(chColour, 1.f);
}