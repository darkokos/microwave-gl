#version 330 core

out vec4 outColour;

uniform bool isOn;

void main() {
	if (isOn)
		outColour = vec4(1., 1., 0., 1.);
	else
		outColour = vec4(0., 0., 0., 1.);
}