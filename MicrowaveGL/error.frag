#version 330 core

in vec2 chTexture;

out vec4 outColour;

uniform sampler2D uTexture;
uniform bool uIsOn;

void main() {
	if (uIsOn)
		outColour = texture(uTexture, chTexture);
	else
		outColour = vec4(0., 0., 0., 1.);
}