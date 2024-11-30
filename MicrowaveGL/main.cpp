#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>
#include <thread>

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
	Transparent,
	Break,
};

struct Button {
	vec2 position;
	unsigned int width;
	unsigned int height;
	NumpadCharacter value;
};

struct Numpad {
	vec2 normalisedPosition;
	float normalisedWidth;
	float normalisedHeight;
	vec2 position;
	unsigned int width;
	unsigned int height;
	Button buttons[17];
};

constexpr auto WINDOW_TITLE = "Microwave";
constexpr double targetRenderTime = (double) 1 / 60;
constexpr double breakdownTime = 1.8;

int windowWidth;
int windowHeight;

Numpad numpad;
unsigned int timerValue[4] = { 0, 0, 0, 0 };
bool isOpen;
bool isRunning;
bool isTransparent = true;
bool isBreaking;
bool isBroken;
bool isRepairing;

void initNumpad(const vec2&, const float&, const float&);
void updateNumpad();
const unsigned int getWindowDimension(const bool&);
void framebufferSizeCallback(GLFWwindow *const, const int, const int);
void insertIntoTimer(const unsigned int&, const size_t&);
void decrementTimerValue(const size_t&);
bool isTimerValueZero();
void resetTimerValue();
void handleButtonPress(const NumpadCharacter&);
const bool isWithin(const Button&, const double&, const double&);
void mouseClickCallback(GLFWwindow *const, const int, const int, const int);
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

template <size_t N, size_t M>
void setupGlBuffersForMaskObject(const float (&)[N], const unsigned int (&)[M], unsigned int&, unsigned int&, unsigned int&);

void processWindowInput(GLFWwindow *const);
void teardownGlElementBuffers(const unsigned int&, const unsigned int&, const unsigned int&);
void teardownGlArrayBuffers(const unsigned int&, const unsigned int&);

void initNumpad(const vec2& position, const float& normalisedWidth, const float& normalisedHeight) {
	numpad.normalisedPosition = position;
	numpad.normalisedWidth = normalisedWidth;
	numpad.normalisedHeight = normalisedHeight;

	numpad.position = vec2((numpad.normalisedPosition.x + 1.f) * windowWidth / 2.f, (1.f - numpad.normalisedPosition.y) * windowHeight / 2.f);

	numpad.width = (unsigned int) (numpad.normalisedWidth * windowWidth / 2);
	numpad.height = (unsigned int) (numpad.normalisedHeight * windowHeight / 2);

	const unsigned int buttonWidth = numpad.width / 3;
	const unsigned int buttonHeight = numpad.height / 6;
	for (size_t i = 0; i < 17; i++) {
		const float buttonX = numpad.position.x + (i == 16 ? 2 : (i % 3)) * buttonWidth;
		const float buttonY = numpad.position.y + (float) (i / 3) * buttonHeight;
		numpad.buttons[i].position = vec2(buttonX, buttonY);
		numpad.buttons[i].width = buttonWidth;
		numpad.buttons[i].height = buttonHeight;
		numpad.buttons[i].value = static_cast<NumpadCharacter>(i);
	}
}

void updateNumpad() {
	numpad.position = vec2((numpad.normalisedPosition.x + 1.f) * windowWidth / 2.f, (1.f - numpad.normalisedPosition.y) * windowHeight / 2.f);

	numpad.width = (unsigned int) (numpad.normalisedWidth * windowWidth / 2);
	numpad.height = (unsigned int) (numpad.normalisedHeight * windowHeight / 2);

	const unsigned int buttonWidth = numpad.width / 3;
	const unsigned int buttonHeight = numpad.height / 6;
	for (size_t i = 0; i < 17; i++) {
		const float buttonX = numpad.position.x + (i == 16 ? 2 : (i % 3)) * buttonWidth;
		const float buttonY = numpad.position.y + (float) (i / 3) * buttonHeight;
		numpad.buttons[i].position = vec2(buttonX, buttonY);
		numpad.buttons[i].width = buttonWidth;
		numpad.buttons[i].height = buttonHeight;
	}
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
	updateNumpad();
	glViewport(0, 0, width, height);
}

void insertIntoTimer(const unsigned int& digit, const size_t& index) {
	if (!index) {
		timerValue[index] = digit;
		return;
	}
	
	insertIntoTimer(timerValue[index], index - 1);
	timerValue[index] = digit;

	if (timerValue[2] > 5) {
		timerValue[2] -= 6;
		timerValue[1]++;
	}
}

void decrementTimerValue(const size_t& index) {
	if (!index && !timerValue[index]) {
		isRunning = false;
		return;
	}

	if (!timerValue[index]) {
		decrementTimerValue(index - 1);

		if (isRunning)
			timerValue[index] = index != 2 ? 9 : 5;

		return;
	}

	timerValue[index]--;
}

bool isTimerValueZero() {
	for (size_t i = 0; i < 4; i++)
		if (timerValue[i])
			return false;
	
	return true;
}

void resetTimerValue() {
	for (size_t i = 0; i < 4; i++)
		timerValue[i] = 0;
}

void handleButtonPress(const NumpadCharacter& pressedCharacter) {
	switch (pressedCharacter) {
		case One:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(1, 3);
			break;
		case Two:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(2, 3);
			break;
		case Three:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(3, 3);
			break;
		case Four:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(4, 3);
			break;
		case Five:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(5, 3);
			break;
		case Six:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(6, 3);
			break;
		case Seven:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(7, 3);
			break;
		case Eight:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(8, 3);
			break;
		case Nine:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(9, 3);
			break;
		case Open:
			if (isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			isRunning = false;
			isOpen = true;
			break;
		case Zero:
			if (isRunning || isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			insertIntoTimer(0, 3);
			break;
		case Close:
			if (isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			isOpen = false;
			break;
		case Start:
			if (isBroken) {
				isRepairing = true;
				break;
			}

			if (isTimerValueZero() || isOpen || isBreaking || isRepairing)
				break;

			isRunning = true;
			break;
		case Stop:
			if (isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			isRunning = false;
			break;
		case Reset:
			if (isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			isRunning = false;
			resetTimerValue();
			break;
		case Transparent:
			if (isBreaking || isRepairing)
				break;

			if (isBroken) {
				isRepairing = true;
				break;
			}

			isTransparent = !isTransparent;
			break;
		case Break:
			if (isBreaking || isBroken || isRepairing)
				break;

			isBreaking = true;
			break;
	}
}

const bool isWithin(const Button& button, const double& xpos, const double& ypos) {
	return
		xpos >= button.position.x &&
		xpos <= button.position.x + button.width &&
		ypos >= button.position.y &&
		ypos <= button.position.y + button.height;
}

void mouseClickCallback(GLFWwindow* const window, const int button, const int action, const int mods) {
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
		return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	for (size_t i = 0; i < 17; i++) {
		if (isWithin(numpad.buttons[i], xpos, ypos)) {
			handleButtonPress(numpad.buttons[i].value);
			break;
		}
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
	const float step = 2.f * (float) M_PI / segments;

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

template <size_t N, size_t M>
void setupGlBuffersForMaskObject(const float (&vertices)[N], const unsigned int (&indices)[M], unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
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
	windowWidth = getWindowDimension(false), windowHeight = getWindowDimension(true);

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
	glfwSetMouseButtonCallback(window, mouseClickCallback);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const unsigned int basicShader = createShaderProgram("basic.vert", "basic.frag");
	const unsigned int transparentShader = createShaderProgram("transparent.vert", "transparent.frag");
	const unsigned int smokeTransparentShader = createShaderProgram("transparent.vert", "transparent.frag");
	const unsigned int textureShader = createShaderProgram("texture.vert", "texture.frag");
	const unsigned int lampShader = createShaderProgram("lamp.vert", "lamp.frag");
	const unsigned int indicatorShader = createShaderProgram("indicator.vert", "indicator.frag");
	const unsigned int maskShader = createShaderProgram("mask.vert", "mask.frag");
	const unsigned int errorShader = createShaderProgram("error.vert", "error.frag");

	const unsigned int rectangleIndices[] = {
		0, 1, 2,
		1, 2, 3,
	};

	const float secondSmokeFirstCloudVertices[] = {
		.45f,  .10f,	.25f, .25f, .25f, .75f,
		.55f,  .10f,	.25f, .25f, .25f, .75f,
		.45f, -.10f,	.25f, .25f, .25f, .75f,
		.55f, -.10f,	.25f, .25f, .25f, .75f,
	};

	unsigned int secondSmokeFirstCloudVAO, secondSmokeFirstCloudVBO, secondSmokeFirstCloudEBO;

	setupGlBuffersForTransparentObject(secondSmokeFirstCloudVertices, rectangleIndices, secondSmokeFirstCloudVAO, secondSmokeFirstCloudVBO, secondSmokeFirstCloudEBO);

	const float secondSmokeSecondCloudVertices[] = {
		.45f,  .40f,	.25f, .25f, .25f, .50f,
		.65f,  .40f,	.25f, .25f, .25f, .50f,
		.45f,  .00f,	.25f, .25f, .25f, .50f,
		.65f,  .00f,	.25f, .25f, .25f, .50f,
	};

	unsigned int secondSmokeSecondCloudVAO, secondSmokeSecondCloudVBO, secondSmokeSecondCloudEBO;

	setupGlBuffersForTransparentObject(secondSmokeSecondCloudVertices, rectangleIndices, secondSmokeSecondCloudVAO, secondSmokeSecondCloudVBO, secondSmokeSecondCloudEBO);
	
	const float secondSmokeThirdCloudVertices[] = {
		.45f,  1.0f,	.25f, .25f, .25f, .25f,
		.85f,  1.0f,	.25f, .25f, .25f, .25f,
		.45f,  .20f,	.25f, .25f, .25f, .25f,
		.85f,  .20f,	.25f, .25f, .25f, .25f,
	};

	unsigned int secondSmokeThirdCloudVAO, secondSmokeThirdCloudVBO, secondSmokeThirdCloudEBO;

	setupGlBuffersForTransparentObject(secondSmokeThirdCloudVertices, rectangleIndices, secondSmokeThirdCloudVAO, secondSmokeThirdCloudVBO, secondSmokeThirdCloudEBO);

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
		-.3625f, -.10f,		0.f, 1.f,
		 .0125f, -.10f,		1.f, 1.f,
		-.3625f, -.35f,		0.f, 0.f,
		 .0125f, -.35f,		1.f, 0.f,
	};

	unsigned int foodVAO, foodVBO, foodEBO;

	setupGlBuffersForTextureObject(foodVertices, rectangleIndices, foodVAO, foodVBO, foodEBO);

	const float raysVertices[] = {
		-.2000f,  .30f,	1.f, 1.f, 0.f, .5f,
		-.1500f,  .30f,	1.f, 1.f, 0.f, .5f,
		-.3625f, -.35f, 1.f, 1.f, 0.f, .5f,
		 .0125f, -.35f,	1.f, 1.f, 0.f, .5f,
	};

	unsigned int raysVAO, raysVBO, raysEBO;

	setupGlBuffersForTransparentObject(raysVertices, rectangleIndices, raysVAO, raysVBO, raysEBO);

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

	const float firstErrorVertices[] = {
		.25f,  .41f,		0.f, 1.f,
		.29f,  .41f,		1.f, 1.f,
		.25f,  .32f,		0.f, 0.f,
		.29f,  .32f,		1.f, 0.f,
	};

	unsigned int firstErrorVAO, firstErrorVBO, firstErrorEBO;

	setupGlBuffersForTextureObject(firstErrorVertices, rectangleIndices, firstErrorVAO, firstErrorVBO, firstErrorEBO);

	const float secondErrorVertices[] = {
		.29f,  .41f,		0.f, 1.f,
		.33f,  .41f,		1.f, 1.f,
		.29f,  .32f,		0.f, 0.f,
		.33f,  .32f,		1.f, 0.f,
	};

	unsigned int secondErrorVAO, secondErrorVBO, secondErrorEBO;

	setupGlBuffersForTextureObject(secondErrorVertices, rectangleIndices, secondErrorVAO, secondErrorVBO, secondErrorEBO);

	const float thirdErrorVertices[] = {
		.33f, .41f,			0.f, 1.f,
		.37f, .41f,			1.f, 1.f,
		.33f, .32f,			0.f, 0.f,
		.37f, .32f,			1.f, 0.f,
	};

	unsigned int thirdErrorVAO, thirdErrorVBO, thirdErrorEBO;

	setupGlBuffersForTextureObject(thirdErrorVertices, rectangleIndices, thirdErrorVAO, thirdErrorVBO, thirdErrorEBO);

	const float fourthErrorVertices[] = {
		.37f,  .41f,		0.f, 1.f,
		.41f,  .41f,		1.f, 1.f,
		.37f,  .32f,		0.f, 0.f,
		.41f,  .32f,		1.f, 0.f,
	};

	unsigned int fourthErrorVAO, fourthErrorVBO, fourthErrorEBO;

	setupGlBuffersForTextureObject(fourthErrorVertices, rectangleIndices, fourthErrorVAO, fourthErrorVBO, fourthErrorEBO);

	const float fifthErrorVertices[] = {
		.41f,  .41f,		0.f, 1.f,
		.45f,  .41f,		1.f, 1.f,
		.41f,  .32f,		0.f, 0.f,
		.45f,  .32f,		1.f, 0.f,
	};

	unsigned int fifthErrorVAO, fifthErrorVBO, fifthErrorEBO;

	setupGlBuffersForTextureObject(fifthErrorVertices, rectangleIndices, fifthErrorVAO, fifthErrorVBO, fifthErrorEBO);

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

	initNumpad(vec2(numpadVertices[0], numpadVertices[1]), abs(numpadVertices[4] - numpadVertices[0]), abs(numpadVertices[1] - numpadVertices[9]));

	const float firstSmokeFirstCloudVertices[] = {
		-.55f,  .10f,	.25f, .25f, .25f, .75f,
		-.45f,  .10f,	.25f, .25f, .25f, .75f,
		-.55f, -.10f,	.25f, .25f, .25f, .75f,
		-.45f, -.10f,	.25f, .25f, .25f, .75f,
	};

	unsigned int firstSmokeFirstCloudVAO, firstSmokeFirstCloudVBO, firstSmokeFirstCloudEBO;

	setupGlBuffersForTransparentObject(firstSmokeFirstCloudVertices, rectangleIndices, firstSmokeFirstCloudVAO, firstSmokeFirstCloudVBO, firstSmokeFirstCloudEBO);

	const float firstSmokeSecondCloudVertices[] = {
		-.65f,  .40f,	.25f, .25f, .25f, .50f,
		-.45f,  .40f,	.25f, .25f, .25f, .50f,
		-.65f,  .00f,	.25f, .25f, .25f, .50f,
		-.45f,  .00f,	.25f, .25f, .25f, .50f,
	};

	unsigned int firstSmokeSecondCloudVAO, firstSmokeSecondCloudVBO, firstSmokeSecondCloudEBO;

	setupGlBuffersForTransparentObject(firstSmokeSecondCloudVertices, rectangleIndices, firstSmokeSecondCloudVAO, firstSmokeSecondCloudVBO, firstSmokeSecondCloudEBO);

	const float firstSmokeThirdCloudVertices[] = {
		-.85f,  1.0f,	.25f, .25f, .25f, .25f,
		-.45f,  1.0f,	.25f, .25f, .25f, .25f,
		-.85f,  .20f,	.25f, .25f, .25f, .25f,
		-.45f,  .20f,	.25f, .25f, .25f, .25f,
	};

	unsigned int firstSmokeThirdCloudVAO, firstSmokeThirdCloudVBO, firstSmokeThirdCloudEBO;

	setupGlBuffersForTransparentObject(firstSmokeThirdCloudVertices, rectangleIndices, firstSmokeThirdCloudVAO, firstSmokeThirdCloudVBO, firstSmokeThirdCloudEBO);

	const float signatureVertices[] = {
		-1.f,	1.f,		0.f, 1.f,
		-.8f,	1.f,		1.f, 1.f,
		-1.f,	.8f,		0.f, 0.f,
		-.8f,	.8f,		1.f, 0.f,
	};

	unsigned int signatureVAO, signatureVBO, signatureEBO;

	setupGlBuffersForTextureObject(signatureVertices, rectangleIndices, signatureVAO, signatureVBO, signatureEBO);

	const float maskVertices[] = {
		-1.f,  1.f,		0.f, 0.f, 0.f,
		 1.f,  1.f,		0.f, 0.f, 0.f,
		-1.f, -1.f,		0.f, 0.f, 0.f,
		 1.f, -1.f,		0.f, 0.f, 0.f,
	};

	unsigned int maskVAO, maskVBO, maskEBO;

	setupGlBuffersForMaskObject(maskVertices, rectangleIndices, maskVAO, maskVBO, maskEBO);

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
	const unsigned int letterE = loadTexture("res/E.png");
	const unsigned int letterR = loadTexture("res/R.png");
	const unsigned int letterO = loadTexture("res/O.png");

	setupSharpTexture(signature);
	setup8BitTexture(food);
	setup8BitTexture(numpad);
	for (size_t i = 0; i < 10; i++)
		setup8BitTexture(digits[i]);
	setup8BitTexture(letterE);
	setup8BitTexture(letterR);
	setup8BitTexture(letterO);

	double renderStartTime = glfwGetTime();
	double lastTimerChangeTime = glfwGetTime();
	double breakdownBeginningTime = 0;
	bool shouldErrorBeDisplayed = false;
	double lastErrorDisplayChangeTime = 0;
	double repairBeginningTime = 0;

	while (!glfwWindowShouldClose(window)) {
		processWindowInput(window);

		glClearColor(1.f, .33f, .0f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		const double timerAssessmentTime = glfwGetTime();
		if (isRunning) {
			if (timerAssessmentTime - lastTimerChangeTime >= 1) {
				decrementTimerValue(3);
				lastTimerChangeTime = timerAssessmentTime;
			}
		} else lastTimerChangeTime = glfwGetTime();

		if (isBreaking && !breakdownBeginningTime)
			breakdownBeginningTime = glfwGetTime();

		if (isBreaking && renderStartTime - breakdownBeginningTime >= breakdownTime) {
			isBreaking = false;
			isRunning = false;
			breakdownBeginningTime = 0;
			shouldErrorBeDisplayed = false;
			lastErrorDisplayChangeTime = 0;
			isBroken = true;
		}

		if (isRepairing && !repairBeginningTime)
			repairBeginningTime = glfwGetTime();

		if (isRepairing && renderStartTime - repairBeginningTime >= breakdownTime) {
			isRepairing = false;
			repairBeginningTime = 0;
			isBroken = false;
			resetTimerValue();
		}

		if (isBreaking) {
			if (renderStartTime - breakdownBeginningTime >= breakdownTime / 3) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeFirstCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}

			if (renderStartTime - breakdownBeginningTime >= 2 * breakdownTime / 3) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeSecondCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}

			if (renderStartTime - breakdownBeginningTime >= breakdownTime) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeThirdCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		} else if (isBroken) {
			if (isRepairing) {
				if (renderStartTime - repairBeginningTime <= breakdownTime / 3) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(secondSmokeFirstCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}

				if (renderStartTime - repairBeginningTime <= 2 * breakdownTime / 3) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(secondSmokeSecondCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}

				if (renderStartTime - repairBeginningTime <= breakdownTime) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(secondSmokeThirdCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}
			} else {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeFirstCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);

				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeSecondCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);

				glUseProgram(smokeTransparentShader);
				glBindVertexArray(secondSmokeThirdCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}

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

		if (isRunning && !isBroken) {
			glUseProgram(transparentShader);
			glBindVertexArray(raysVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}

		const unsigned int lampShaderUIsOn = glGetUniformLocation(lampShader, "uIsOn");
		glProgramUniform1i(lampShader, lampShaderUIsOn, isRunning && !isBroken);
		glUseProgram(lampShader);
		glBindVertexArray(lampVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (GLsizei) lampVertices.size() / 2);
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
			const unsigned int uIsOpaque = glGetUniformLocation(transparentShader, "uIsOpaque");
			glProgramUniform1i(transparentShader, uIsOpaque, !isTransparent);
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

		if (isBroken) {
			if (renderStartTime - lastErrorDisplayChangeTime >= 1) {
				shouldErrorBeDisplayed = !shouldErrorBeDisplayed;
				lastErrorDisplayChangeTime = glfwGetTime();
			}

			const unsigned int uIsOn = glGetUniformLocation(errorShader, "uIsOn");
			glProgramUniform1i(errorShader, uIsOn, shouldErrorBeDisplayed);

			glUseProgram(errorShader);
			glBindVertexArray(firstErrorVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, letterE);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(errorShader);
			glBindVertexArray(secondErrorVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, letterR);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(errorShader);
			glBindVertexArray(thirdErrorVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, letterR);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(errorShader);
			glBindVertexArray(fourthErrorVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, letterO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(errorShader);
			glBindVertexArray(fifthErrorVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, letterR);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		} else {
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
		}

		const unsigned int indicatorShaderUIsOn = glGetUniformLocation(indicatorShader, "uIsOn");
		glProgramUniform1i(indicatorShader, indicatorShaderUIsOn, isRunning && !isBroken);
		const unsigned int indicatorShaderUTime = glGetUniformLocation(indicatorShader, "uTime");
		glProgramUniform1f(indicatorShader, indicatorShaderUTime, 2 * (GLfloat) glfwGetTime());
		glUseProgram(indicatorShader);
		glBindVertexArray(indicatorVAO);
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

		if (isBreaking) {
			if (renderStartTime - breakdownBeginningTime >= breakdownTime / 3) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeFirstCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}

			if (renderStartTime - breakdownBeginningTime >= 2 * breakdownTime / 3) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeSecondCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}

			if (renderStartTime - breakdownBeginningTime >= breakdownTime) {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeThirdCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		} else if (isBroken) {
			if (isRepairing) {
				if (renderStartTime - repairBeginningTime <= breakdownTime / 3) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(firstSmokeFirstCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}

				if (renderStartTime - repairBeginningTime <= 2 * breakdownTime / 3) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(firstSmokeSecondCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}

				if (renderStartTime - repairBeginningTime <= breakdownTime) {
					glUseProgram(smokeTransparentShader);
					glBindVertexArray(firstSmokeThirdCloudVAO);
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
					glUseProgram(0);
				}
			} else {
				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeFirstCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);

				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeSecondCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);

				glUseProgram(smokeTransparentShader);
				glBindVertexArray(firstSmokeThirdCloudVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
				glBindVertexArray(0);
				glUseProgram(0);
			}
		}

		glUseProgram(textureShader);
		glBindVertexArray(signatureVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, signature);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		if (isBreaking || isBroken) {
			const unsigned int maskShaderUTransparencyCoefficent = glGetUniformLocation(maskShader, "uTransparencyCoefficient");
			glProgramUniform1f(
				maskShader,
				maskShaderUTransparencyCoefficent,
				isRepairing ? (GLfloat) (breakdownTime - (renderStartTime - repairBeginningTime)) / 3 : (GLfloat) (isBroken ? breakdownTime : (renderStartTime - breakdownBeginningTime)) / 3
			);
			glUseProgram(maskShader);
			glBindVertexArray(maskVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glUseProgram(0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		double renderTime = glfwGetTime() - renderStartTime;
		if (renderTime < targetRenderTime)
			this_thread::sleep_for(chrono::nanoseconds((long long) ((targetRenderTime - renderTime) * 1'000'000'000)));

		renderStartTime = glfwGetTime();
	}

	teardownGlElementBuffers(secondSmokeFirstCloudVAO, secondSmokeFirstCloudVBO, secondSmokeFirstCloudEBO);
	teardownGlElementBuffers(secondSmokeSecondCloudVAO, secondSmokeSecondCloudVBO, secondSmokeSecondCloudEBO);
	teardownGlElementBuffers(secondSmokeThirdCloudVAO, secondSmokeThirdCloudVBO, secondSmokeThirdCloudEBO);
	teardownGlElementBuffers(bodyVAO, bodyVBO, bodyEBO);
	teardownGlElementBuffers(legOneVAO, legOneVBO, legOneEBO);
	teardownGlElementBuffers(legTwoVAO, legTwoVBO, legTwoEBO);
	teardownGlElementBuffers(doorSlitVAO, doorSlitVBO, doorSlitEBO);
	teardownGlElementBuffers(interiorVAO, interiorVBO, interiorEBO);
	teardownGlElementBuffers(foodVAO, foodVBO, foodEBO);
	teardownGlElementBuffers(raysVAO, raysVBO, raysEBO);
	teardownGlArrayBuffers(lampVAO, lampVBO);
	teardownGlElementBuffers(doorOpenVAO, doorOpenVBO, doorOpenEBO);
	teardownGlElementBuffers(glassVAO, glassVBO, glassEBO);
	teardownGlElementBuffers(handleVAO, handleVBO, handleEBO);
	teardownGlElementBuffers(firstErrorVAO, firstErrorVBO, firstErrorEBO);
	teardownGlElementBuffers(secondErrorVAO, secondErrorVBO, secondErrorEBO);
	teardownGlElementBuffers(thirdErrorVAO, thirdErrorVBO, thirdErrorEBO);
	teardownGlElementBuffers(fourthErrorVAO, fourthErrorVBO, fourthErrorEBO);
	teardownGlElementBuffers(fifthErrorVAO, fifthErrorVBO, fifthErrorEBO);
	teardownGlElementBuffers(firstDigitVAO, firstDigitVBO, firstDigitEBO);
	teardownGlElementBuffers(secondDigitVAO, secondDigitVBO, secondDigitEBO);
	teardownGlElementBuffers(separatorBackgroundVAO, separatorBackgroundVBO, separatorBackgroundVBO);
	teardownGlElementBuffers(firstSeparatorVAO, firstSeparatorVBO, firstSeparatorEBO);
	teardownGlElementBuffers(secondSeparatorVAO, secondSeparatorVBO, secondSeparatorEBO);
	teardownGlElementBuffers(thirdDigitVAO, thirdDigitVBO, thirdDigitEBO);
	teardownGlElementBuffers(fourthDigitVAO, fourthDigitVBO, fourthDigitEBO);
	teardownGlElementBuffers(indicatorVAO, indicatorVBO, indicatorEBO);
	teardownGlElementBuffers(numpadVAO, numpadVBO, numpadEBO);
	teardownGlElementBuffers(firstSmokeFirstCloudVAO, firstSmokeFirstCloudVBO, firstSmokeFirstCloudEBO);
	teardownGlElementBuffers(firstSmokeSecondCloudVAO, firstSmokeSecondCloudVBO, firstSmokeSecondCloudEBO);
	teardownGlElementBuffers(firstSmokeThirdCloudVAO, firstSmokeThirdCloudVBO, firstSmokeThirdCloudEBO);
	teardownGlElementBuffers(signatureVAO, signatureVBO, signatureEBO);
	teardownGlElementBuffers(maskVAO, maskVBO, maskEBO);
	glDeleteProgram(basicShader);
	glfwTerminate();

	return 0;
}