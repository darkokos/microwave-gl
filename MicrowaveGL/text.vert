#version 330 core

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTextureCoordinate;

out vec2 chTextureCoordinate;

uniform mat4 uProjection;

void main() {
	gl_Position = uProjection * vec4(inPosition.xy, 0., 1.);
	chTextureCoordinate = inTextureCoordinate;
}