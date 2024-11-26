#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

const char* WINDOW_TITLE = "Microwave";

const unsigned int getWindowDimension(const bool&);
void framebufferSizeCallback(GLFWwindow *const, const int, const int);
void processWindowInput(GLFWwindow *const);
void setupGlBuffers(unsigned int&, unsigned int&, unsigned int&);
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

void setupGlBuffers(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
	const float vertices[] = {
		-.5f, -.5f, .0f,
		-.5f, .5f, .0f,
		.5f, -.5f, .0f,
		.5f, .5f, .0f,
	};

	const unsigned int indices[] = {
		0, 1, 2,
		1, 2, 3,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

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

	const unsigned int shaderProgram = createShaderProgram("basic.vert", "basic.frag");

	unsigned int VAO, VBO, EBO;
	setupGlBuffers(VAO, VBO, EBO);

	while (!glfwWindowShouldClose(window)) {
		processWindowInput(window);

		glClearColor(.54f, .81f, .0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	teardownGlBuffers(VAO, VBO, EBO);
	glDeleteProgram(shaderProgram);
	glfwTerminate();

	return 0;
}