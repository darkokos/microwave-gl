#version 330 core

out vec4 outColour;

uniform bool uIsOn;

void main() {
	if (uIsOn)
		outColour = vec4(1., 1., 0., 1.);
	else
		outColour = vec4(0., 0., 0., 1.);
}