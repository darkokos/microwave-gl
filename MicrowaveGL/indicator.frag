#version 330 core

in vec3 chColour;

out vec4 outColour;

uniform bool isOn;

void main() {
	if (isOn)
		outColour = vec4(chColour.xyz, 1.);
	else
		outColour = vec4(0., 0., 0., 1.);
}