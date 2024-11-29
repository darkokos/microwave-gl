#version 330 core

in vec4 chColour;

out vec4 outColour;

uniform bool uIsOpaque;

void main() {
	if (uIsOpaque)
		outColour = vec4(chColour.xyz, 1.);
	else
		outColour =	chColour;
}
