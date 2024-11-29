#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>

using namespace std;
using namespace glm;

enum NumpadCharacter {
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Open,
	Zero,
	Close,
	Start,
	Stop,
	Reset,
	Break,
};

struct Button {
	vec2 position;
	unsigned int width;
	unsigned int height;
	NumpadCharacter value;
};

constexpr auto WINDOW_TITLE = "Microwave";

int windowWidth;
int windowHeight;

Button buttons[16];
unsigned int timerValue[4] = { 0, 0, 0, 0 };
bool isOpen;
bool isRunning;
bool isBroken;

void initButtons(const unsigned int&, const unsigned int&);
const unsigned int getWindowDimension(const bool&);
void framebufferSizeCallback(GLFWwindow *const, const int, const int);
void parseButtonPress(const NumpadCharacter&);
const unsigned int compileShader(const GLenum, const char*);
const unsigned int createShaderProgram(const char*, const char*);
const unsigned int loadTexture(const char*);
void setup8BitTexture(const unsigned int&);
void setupSharpTexture(const unsigned int&);
const vector<float> createCircleVertices(const float&, const float&, const float&, const unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForBasicObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForTransparentObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForTextureObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

void setupGlBuffersForLampObject(const vector<float>&, unsigned int&, unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForIndicatorObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForTextObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

void processWindowInput(GLFWwindow *const);
void teardownGlElementBuffers(const unsigned int&, const unsigned int&, const unsigned int&);
void teardownGlArrayBuffers(const unsigned int&, const unsigned int&);

void initButtons(const unsigned int& windowWidth, const unsigned int& windowHeight) {
	const float buttonWidth = 32.f, buttonHeight = 32.f;
}

const unsigned int getWindowDimension(const bool& is_height) {
	int dimension;
	string input;
	while (true) {
		try {
			cout << "Input the desired window " << (is_height ? "height" : "width") << " [>= 0]: ";
			cin >> input;

			dimension = stoi(input);
			if (dimension <= 0) {
				continue;
			}

			break;
		}
		catch (const exception&) {
			continue;
		}
	}

	return dimension;
}

void framebufferSizeCallback(GLFWwindow *const window, const int width, const int height) {
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
}

void parseButtonPress(const NumpadCharacter& pressedCharacter) {
	if (isBroken && pressedCharacter != Break)
		isBroken = false;

	switch (pressedCharacter) {
		case One: break;
		case Two: break;
		case Three: break;
		case Four: break;
		case Five: break;
		case Six: break;
		case Seven: break;
		case Eight: break;
		case Nine: break;
		case Open: isOpen = true;
		case Zero: break;
		case Close: isOpen = false;
		case Start: isRunning = true;
		case Stop: isRunning = false;
		case Reset: isRunning = false;
		case Break: isBroken = true;
	}
}

const unsigned int compileShader(const GLenum shaderType, const char* shaderPath) {
	ifstream shaderFile(shaderPath);
	stringstream shaderStream;

	if (!shaderFile.is_open()) {
		cerr << "Failed to open shader file at path: " << shaderPath << endl;
		return 0;
	}

	shaderStream << shaderFile.rdbuf();
	shaderFile.close();

	const string shaderCodeStr = shaderStream.str();
	const char* shaderCode = shaderCodeStr.c_str();

	const unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, nullptr);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		const size_t infoLogSize = 512;
		char infoLog[infoLogSize];
		glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog);

		cerr << "Error compiling " << (shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader:\n" << infoLog << endl;
	}
	
	return shader;
}

const unsigned int createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
	const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderPath);
	const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderPath);
	const unsigned int shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glValidateProgram(shaderProgram);

	int success;
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		const size_t infoLogSize = 512;
		char infoLog[infoLogSize];
		glGetProgramInfoLog(shaderProgram, infoLogSize, nullptr, infoLog);

		cerr << "Error creating the shader program:\n" << infoLog << endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

const unsigned int loadTexture(const char* path) {
	int width, height, numberOfChannels;
	unsigned char* textureData = stbi_load(path, &width, &height, &numberOfChannels, 0);
	if (textureData == nullptr) {
		cerr << "Failed to load texture at path: " << path << endl;
		stbi_image_free(textureData);
		return 0;
	}

	stbi__vertical_flip(textureData, width, height, numberOfChannels);

	int colourSpace;
	switch (numberOfChannels) {
		case 1: colourSpace = GL_RED; break;
		case 3: colourSpace = GL_RGB; break;
		case 4: colourSpace = GL_RGBA; break;
		default: colourSpace = GL_RGB; break;
	}

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, colourSpace, width, height, 0, colourSpace, GL_UNSIGNED_BYTE, textureData);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(textureData);

	return texture;
}

void setup8BitTexture (const unsigned int& texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void setupSharpTexture(const unsigned int& texture) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

const vector<float> createCircleVertices(const float& centerX, const float& centerY, const float& radius, const unsigned int& segments) {
	vector<float> vertices;
	const float step = 2.f * M_PI / segments;

	vertices.push_back(centerX);
	vertices.push_back(centerY);

	for (size_t i = 0; i <= segments; i++) {
		float angle = step * i;
		vertices.push_back((radius - .023) * cos(angle) + centerX);
		vertices.push_back(radius * sin(angle) + centerY);
	}

	return vertices;
}

template <size_t N, size_t M>
void setupGlBuffersForBasicObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template <size_t N, size_t M>
void setupGlBuffersForTransparentObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template <size_t N, size_t M>
void setupGlBuffersForTextureObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void setupGlBuffersForLampObject(const vector<float>& vertices, unsigned int& VAO, unsigned int& VBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template <size_t N, size_t M>
void setupGlBuffersForIndicatorObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

template <size_t N, size_t M>
void setupGlBuffersForTextObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices), indices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void processWindowInput(GLFWwindow *const window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void teardownGlElementBuffers(const unsigned int& VAO, const unsigned int& VBO, const unsigned int& EBO) {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

void teardownGlArrayBuffers(const unsigned int& VAO, const unsigned int& VBO) {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

int main(void) {
	const int windowWidth = getWindowDimension(false), windowHeight = getWindowDimension(true);

	if (!glfwInit()) {
		cerr << "Failed to initialise GLFW." << endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, WINDOW_TITLE, nullptr, nullptr);
	if (!window) {
		cerr << "Failed to create a GLFW window." << endl;
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialise GLEW." << endl;
		return 3;
	}

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const unsigned int basicShader = createShaderProgram("basic.vert", "basic.frag");
	const unsigned int transparentShader = createShaderProgram("transparent.vert", "transparent.frag");
	const unsigned int textureShader = createShaderProgram("texture.vert", "texture.frag");
	const unsigned int lampShader = createShaderProgram("lamp.vert", "lamp.frag");
	const unsigned int indicatorShader = createShaderProgram("indicator.vert", "indicator.frag");
	const unsigned int textShader = createShaderProgram("text.vert", "text.frag");

	const unsigned int rectangleIndices[] = {
		0, 1, 2,
		1, 2, 3,
	};

	const float bodyVertices[] = {
		-.5f, -.5f,		.125f, .125f, .125f,
		-.5f,  .5f,		.125f, .125f, .125f,
		 .5f, -.5f,		.125f, .125f, .125f,
		 .5f,  .5f,		.125f, .125f, .125f,
	};

	unsigned int bodyVAO, bodyVBO, bodyEBO;

	setupGlBuffersForBasicObject(bodyVertices, rectangleIndices, bodyVAO, bodyVBO, bodyEBO);

	const float legOneVertices[] = {
		-.4f, -.5f,		.25f, .25f, .25f,
		-.3f, -.5f,		.25f, .25f, .25f,
		-.4f, -.6f,		.25f, .25f, .25f,
		-.3f, -.6f,		.25f, .25f, .25f,
	};

	unsigned int legOneVAO, legOneVBO, legOneEBO;

	setupGlBuffersForBasicObject(legOneVertices, rectangleIndices, legOneVAO, legOneVBO, legOneEBO);

	const float legTwoVertices[] = {
		.4f, -.5f,		.25f, .25f, .25f,
		.3f, -.5f,		.25f, .25f, .25f,
		.4f, -.6f,		.25f, .25f, .25f,
		.3f, -.6f,		.25f, .25f, .25f,
	};

	unsigned int legTwoVAO, legTwoVBO, legTwoEBO;

	setupGlBuffersForBasicObject(legTwoVertices, rectangleIndices, legTwoVAO, legTwoVBO, legTwoEBO);

	const float doorSlitVertices[] = {
		.19f,  .5f,		0.f, 0.f, 0.f,
		.20f,  .5f,		0.f, 0.f, 0.f,
		.19f, -.5f,		0.f, 0.f, 0.f,
		.20f, -.5f,		0.f, 0.f, 0.f,
	};

	unsigned int doorSlitVAO, doorSlitVBO, doorSlitEBO;

	setupGlBuffersForBasicObject(doorSlitVertices, rectangleIndices, doorSlitVAO, doorSlitVBO, doorSlitEBO);

	const float interiorVertices[] = {
		-.475f,  .40f,	.5f, .5f, .5f,
		 .175f,  .40f,	.5f, .5f, .5f,
		-.475f, -.40f,	.5f, .5f, .5f,
		 .175f, -.40f,	.5f, .5f, .5f,
	};

	unsigned int interiorVAO, interiorVBO, interiorEBO;

	setupGlBuffersForBasicObject(interiorVertices, rectangleIndices, interiorVAO, interiorVBO, interiorEBO);

	const float foodVertices[] = {
		-.3125f, -.10f,		0.f, 1.f,
		 .0125f, -.10f,		1.f, 1.f,
		-.3125f, -.35f,		0.f, 0.f,
		 .0125f, -.35f,		1.f, 0.f,
	};

	unsigned int foodVAO, foodVBO, foodEBO;

	setupGlBuffersForTextureObject(foodVertices, rectangleIndices, foodVAO, foodVBO, foodEBO);

	const float centerX = -.175f;
	const float centerY = .3f;
	const float radius = .05f;
	const unsigned int segments = 100;
	vector<float> lampVertices = createCircleVertices(centerX, centerY, radius, segments);

	unsigned int lampVAO, lampVBO;

	setupGlBuffersForLampObject(lampVertices, lampVAO, lampVBO);

	const float doorOpenVertices[] = {
		-.50f,  .5f,		.1f, .1f, .1f,
		-.45f,  .5f,		.1f, .1f, .1f,
		-.50f, -.5f,		.1f, .1f, .1f,
		-.45f, -.5f,		.1f, .1f, .1f,
	};

	unsigned int doorOpenVAO, doorOpenVBO, doorOpenEBO;

	setupGlBuffersForBasicObject(doorOpenVertices, rectangleIndices, doorOpenVAO, doorOpenVBO, doorOpenEBO);

	const float glassVertices[] = {
		-.475f,  .40f,	0.f, .5f, 1.f, .5f,
		 .175f,  .40f,	0.f, .5f, 1.f, .5f,
		-.475f, -.40f,	0.f, .5f, 1.f, .5f,
		 .175f, -.40f,	0.f, .5f, 1.f, .5f,
	};

	unsigned int glassVAO, glassVBO, glassEBO;

	setupGlBuffersForTransparentObject(glassVertices, rectangleIndices, glassVAO, glassVBO, glassEBO);

	const float handleVertices[] = {
		.10f,  .45f,	.25f, .25f, .25f,
		.15f,  .45f,	.25f, .25f, .25f,
		.10f, -.45f,	.25f, .25f, .25f,
		.15f, -.45f,	.25f, .25f, .25f,
	};

	unsigned int handleVAO, handleVBO, handleEBO;

	setupGlBuffersForBasicObject(handleVertices, rectangleIndices, handleVAO, handleVBO, handleEBO);

	const float firstDigitVertices[] = {
		.25f,  .41f,		0.f, 1.f,
		.29f,  .41f,		1.f, 1.f,
		.25f,  .32f,		0.f, 0.f,
		.29f,  .32f,		1.f, 0.f,
	};

	unsigned int firstDigitVAO, firstDigitVBO, firstDigitEBO;

	setupGlBuffersForTextureObject(firstDigitVertices, rectangleIndices, firstDigitVAO, firstDigitVBO, firstDigitEBO);

	const float secondDigitVertices[] = {
		.29f,  .41f,		0.f, 1.f,
		.33f,  .41f,		1.f, 1.f,
		.29f,  .32f,		0.f, 0.f,
		.33f,  .32f,		1.f, 0.f,
	};

	unsigned int secondDigitVAO, secondDigitVBO, secondDigitEBO;

	setupGlBuffersForTextureObject(secondDigitVertices, rectangleIndices, secondDigitVAO, secondDigitVBO, secondDigitEBO);

	const float separatorBackgroundVertices[] = {
		.33f, .41f,		0.f, 0.f, 0.f,
		.37f, .41f,		0.f, 0.f, 0.f,
		.33f, .32f,		0.f, 0.f, 0.f,
		.37f, .32f,		0.f, 0.f, 0.f,
	};

	unsigned int separatorBackgroundVAO, separatorBackgroundVBO, separatorBackgroundEBO;

	setupGlBuffersForBasicObject(separatorBackgroundVertices, rectangleIndices, separatorBackgroundVAO, separatorBackgroundVBO, separatorBackgroundEBO);

	const float firstSeparatorVertices[] = {
		.3475f, .39f, 	1.f, 1.f, 1.f,
		.3525f, .39f,	1.f, 1.f, 1.f,
		.3475f, .38f, 	1.f, 1.f, 1.f,
		.3525f, .38f,	1.f, 1.f, 1.f,
	};

	unsigned int firstSeparatorVAO, firstSeparatorVBO, firstSeparatorEBO;

	setupGlBuffersForBasicObject(firstSeparatorVertices, rectangleIndices, firstSeparatorVAO, firstSeparatorVBO, firstSeparatorEBO);

	const float secondSeparatorVertices[] = {
		.3475f, .36f,	1.f, 1.f, 1.f,
		.3525f, .36f,	1.f, 1.f, 1.f,
		.3475f, .35f,	1.f, 1.f, 1.f,
		.3525f, .35f,	1.f, 1.f, 1.f,
	};

	unsigned int secondSeparatorVAO, secondSeparatorVBO, secondSeparatorEBO;

	setupGlBuffersForBasicObject(secondSeparatorVertices, rectangleIndices, secondSeparatorVAO, secondSeparatorVBO, secondSeparatorEBO);

	const float thirdDigitVertices[] = {
		.37f,  .41f,		0.f, 1.f,
		.41f,  .41f,		1.f, 1.f,
		.37f,  .32f,		0.f, 0.f,
		.41f,  .32f,		1.f, 0.f,
	};

	unsigned int thirdDigitVAO, thirdDigitVBO, thirdDigitEBO;

	setupGlBuffersForTextureObject(thirdDigitVertices, rectangleIndices, thirdDigitVAO, thirdDigitVBO, thirdDigitEBO);

	const float fourthDigitVertices[] = {
		.41f,  .41f,		0.f, 1.f,
		.45f,  .41f,		1.f, 1.f,
		.41f,  .32f,		0.f, 0.f,
		.45f,  .32f,		1.f, 0.f,
	};

	unsigned int fourthDigitVAO, fourthDigitVBO, fourthDigitEBO;

	setupGlBuffersForTextureObject(fourthDigitVertices, rectangleIndices, fourthDigitVAO, fourthDigitVBO, fourthDigitEBO);

	const float indicatorVertices[] = {
		.34f,  .23f,	.75f, 0.f, 0.f,
		.36f,  .23f,	.75f, 0.f, 0.f,
		.34f,  .19f,	.75f, 0.f, 0.f,
		.36f,  .19f,	.75f, 0.f, 0.f,
	};

	unsigned int indicatorVAO, indicatorVBO, indicatorEBO;

	setupGlBuffersForIndicatorObject(indicatorVertices, rectangleIndices, indicatorVAO, indicatorVBO, indicatorEBO);

	const float numpadVertices[] = {
		.25f,  .10f,		0.f, 1.f,
		.45f,  .10f,		1.f, 1.f,
		.25f, -.45f,		0.f, 0.f,
		.45f, -.45f,		1.f, 0.f,
	};

	unsigned int numpadVAO, numpadVBO, numpadEBO;

	setupGlBuffersForTextureObject(numpadVertices, rectangleIndices, numpadVAO, numpadVBO, numpadEBO);

	// NOTE: Last object to be buffered
	const float signatureVertices[] = {
		-1.f,	1.f,		0.f, 1.f,
		-.8f,	1.f,		1.f, 1.f,
		-1.f,	.8f,		0.f, 0.f,
		-.8f,	.8f,		1.f, 0.f,
	};

	unsigned int signatureVAO, signatureVBO, signatureEBO;

	setupGlBuffersForTextureObject(signatureVertices, rectangleIndices, signatureVAO, signatureVBO, signatureEBO);

	// NOTE: After all objects have been buffered
	const unsigned int signature = loadTexture("res/signature.png");
	const unsigned int food = loadTexture("res/food.png");
	const unsigned int numpad = loadTexture("res/numpad.png");
	const unsigned int digits[10] = {
		loadTexture("res/0.png"),
		loadTexture("res/1.png"),
		loadTexture("res/2.png"),
		loadTexture("res/3.png"),
		loadTexture("res/4.png"),
		loadTexture("res/5.png"),
		loadTexture("res/6.png"),
		loadTexture("res/7.png"),
		loadTexture("res/8.png"),
		loadTexture("res/9.png"),
	};

	setupSharpTexture(signature);
	setup8BitTexture(food);
	setup8BitTexture(numpad);
	for (size_t i = 0; i < 10; i++)
		setup8BitTexture(digits[i]);

	while (!glfwWindowShouldClose(window)) {
		processWindowInput(window);

		glClearColor(1.f, .33f, .0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(basicShader);
		glBindVertexArray(bodyVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(legOneVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(legTwoVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(doorSlitVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(interiorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(textureShader);
		glBindVertexArray(foodVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, food);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(lampShader);
		glBindVertexArray(lampVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, lampVertices.size() / 2);
		glBindVertexArray(0);
		glUseProgram(0);

		if (isOpen) {
			glUseProgram(basicShader);
			glBindVertexArray(doorOpenVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}
		else {
			glUseProgram(transparentShader);
			glBindVertexArray(glassVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(basicShader);
			glBindVertexArray(handleVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}

		glUseProgram(textureShader);
		glBindVertexArray(firstDigitVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, digits[timerValue[0]]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(textureShader);
		glBindVertexArray(secondDigitVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, digits[timerValue[1]]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(separatorBackgroundVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(firstSeparatorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(basicShader);
		glBindVertexArray(secondSeparatorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(textureShader);
		glBindVertexArray(thirdDigitVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, digits[timerValue[2]]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(textureShader);
		glBindVertexArray(fourthDigitVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, digits[timerValue[3]]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(textureShader);
		glBindVertexArray(numpadVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, numpad);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glUseProgram(indicatorShader);
		glBindVertexArray(indicatorVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		// NOTE: Always draw last
		glUseProgram(textureShader);
		glBindVertexArray(signatureVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, signature);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	teardownGlElementBuffers(bodyVAO, bodyVBO, bodyEBO);
	teardownGlElementBuffers(legOneVAO, legOneVBO, legOneEBO);
	teardownGlElementBuffers(legTwoVAO, legTwoVBO, legTwoEBO);
	teardownGlElementBuffers(doorSlitVAO, doorSlitVBO, doorSlitEBO);
	teardownGlElementBuffers(interiorVAO, interiorVBO, interiorEBO);
	teardownGlElementBuffers(foodVAO, foodVBO, foodEBO);
	teardownGlArrayBuffers(lampVAO, lampVBO);
	teardownGlElementBuffers(doorOpenVAO, doorOpenVBO, doorOpenEBO);
	teardownGlElementBuffers(glassVAO, glassVBO, glassEBO);
	teardownGlElementBuffers(handleVAO, handleVBO, handleEBO);
	teardownGlElementBuffers(firstDigitVAO, firstDigitVBO, firstDigitEBO);
	teardownGlElementBuffers(secondDigitVAO, secondDigitVBO, secondDigitEBO);
	teardownGlElementBuffers(separatorBackgroundVAO, separatorBackgroundVBO, separatorBackgroundVBO);
	teardownGlElementBuffers(firstSeparatorVAO, firstSeparatorVBO, firstSeparatorEBO);
	teardownGlElementBuffers(secondSeparatorVAO, secondSeparatorVBO, secondSeparatorEBO);
	teardownGlElementBuffers(thirdDigitVAO, thirdDigitVBO, thirdDigitEBO);
	teardownGlElementBuffers(fourthDigitVAO, fourthDigitVBO, fourthDigitEBO);
	teardownGlElementBuffers(numpadVAO, numpadVBO, numpadEBO);
	teardownGlElementBuffers(indicatorVAO, indicatorVBO, indicatorEBO);

	// NOTE: Always release last
	teardownGlElementBuffers(signatureVAO, signatureVBO, signatureEBO);
	glDeleteProgram(basicShader);
	glfwTerminate();

	return 0;
}