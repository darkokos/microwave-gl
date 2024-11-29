#version 330 core

in vec3 chColour;

out vec4 outColour;

uniform bool uIsOn;

void main() {
	if (uIsOn)
		outColour = vec4(chColour.xyz, 1.);
	else
		outColour = vec4(0., 0., 0., 1.);
}