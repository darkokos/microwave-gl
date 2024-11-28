#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

const char* WINDOW_TITLE = "Microwave";

const unsigned int getWindowDimension(const bool&);
void framebufferSizeCallback(GLFWwindow *const, const int, const int);
void processWindowInput(GLFWwindow *const);
const unsigned int loadTexture(const char*);
void setup8BitTexture(const unsigned int&);
void setupSharpTexture(const unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForBasicObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

template <size_t N, size_t M>
void setupGlBuffersForTextureObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

const unsigned int compileShader(const GLenum, const char*);
const unsigned int createShaderProgram(const char*, const char*);
void teardownGlBuffers(const unsigned int&, const unsigned int&, const unsigned int&);

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
	glViewport(0, 0, width, height);
}

void processWindowInput(GLFWwindow *const window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
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

template <size_t N, size_t M>
void setupGlBuffersForBasicObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
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
void setupGlBuffersForTextureObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

void teardownGlBuffers(const unsigned int& VAO, const unsigned int& VBO, const unsigned int& EBO) {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

int main(void) {
	const int width = getWindowDimension(false), height = getWindowDimension(true);

	if (!glfwInit()) {
		cerr << "Failed to initialise GLFW." << endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, WINDOW_TITLE, nullptr, nullptr);
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

	const unsigned int basicShader = createShaderProgram("basic.vert", "basic.frag");
	const unsigned int textureShader = createShaderProgram("texture.vert", "texture.frag");

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

	const float handleVertices[] = {
		.10f,  .45f,	.25f, .25f, .25f,
		.15f,  .45f,	.25f, .25f, .25f,
		.10f, -.45f,	.25f, .25f, .25f,
		.15f, -.45f,	.25f, .25f, .25f,
	};

	unsigned int handleVAO, handleVBO, handleEBO;

	setupGlBuffersForBasicObject(handleVertices, rectangleIndices, handleVAO, handleVBO, handleEBO);

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

	setupSharpTexture(signature);
	setup8BitTexture(food);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

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

		glUseProgram(basicShader);
		glBindVertexArray(handleVAO);
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

	teardownGlBuffers(bodyVAO, bodyVBO, bodyEBO);
	teardownGlBuffers(legOneVAO, legOneVBO, legOneEBO);
	teardownGlBuffers(legTwoVAO, legTwoVBO, legTwoEBO);
	teardownGlBuffers(doorSlitVAO, doorSlitVBO, doorSlitEBO);
	teardownGlBuffers(interiorVAO, interiorVBO, interiorEBO);
	teardownGlBuffers(foodVAO, foodVBO, foodEBO);
	teardownGlBuffers(handleVAO, handleVBO, handleEBO);

	// NOTE: Always release last
	teardownGlBuffers(signatureVAO, signatureVBO, signatureEBO);
	glDeleteProgram(basicShader);
	glfwTerminate();

	return 0;
}