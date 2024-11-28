#version 330 core

in vec2 chTexture;

out vec4 outColour;

uniform sampler2D uTexture;

void main() {
	outColour = texture(uTexture, chTexture);
}