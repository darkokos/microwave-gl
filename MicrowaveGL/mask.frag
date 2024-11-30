#version 330 core

in vec3 chColour;

out vec4 outColour;

uniform float uTransparencyCoefficient;

void main() {
	outColour = vec4(chColour.xyz, sin(uTransparencyCoefficient));
}
