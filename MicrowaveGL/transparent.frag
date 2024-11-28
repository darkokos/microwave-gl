#version 330 core

in vec4 chColour;

out vec4 outColour;

uniform bool isOpaque;

void main() {
	if (isOpaque)
		outColour = vec4(chColour.xyz, 0.);
	else
		outColour =	chColour;
}
