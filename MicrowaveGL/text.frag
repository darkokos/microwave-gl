#version 330 core

in vec2 chTextureCoordinate;

out vec4 outColour;

uniform sampler2D uTexture;

void main() {
	outColour = vec4(1., 1., 1., texture(uTexture, chTextureCoordinate).r);
}
