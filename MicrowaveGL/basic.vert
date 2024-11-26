#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColour;

out vec3 colour;

void main() {
	gl_Position = vec4(aPosition, 1.);
	colour = aColour;
}