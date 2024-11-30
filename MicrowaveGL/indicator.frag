#version 330 core

in vec3 chColour;

out vec4 outColour;

uniform bool uIsOn;
uniform float uTime;

void main() {
	if (uIsOn)
		outColour = vec4(sin(uTime) * chColour.x, sin(uTime) * chColour.y, sin(uTime + 3.14159) * chColour.z, 1.);
	else
		outColour = vec4(0., 0., 0., 1.);
}