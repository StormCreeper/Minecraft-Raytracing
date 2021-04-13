#pragma once
#define GLEW_STATIC
#include "utils.h"
#include <chrono>
#include "Camera.h"
#include "VoxelTexture.h"
#include <GLFW/glfw3.h>

class Renderer {
	int WIDTH, HEIGHT;
	GLFWwindow* window_;
public:
	float dt;
	std::chrono::steady_clock::time_point last_time;

	char title[33] = "Raytraced Minecraft - 0000 FPS";
public:
	bool b_paused;

	//TEXTURES

	// OBJECTS

	unsigned int shader;
	unsigned int vbo, vao;

	unsigned int cube_texture;

	Camera camera;

	VoxelTexture vt;

	char* shader_error;

public:
	Renderer() = default;

	Renderer(const int w, const int h) {
		WIDTH = w;
		HEIGHT = h;
		last_time = std::chrono::high_resolution_clock::now();
	}

public: // GETTERS AND SETTERS
	Camera* getCameraPtr();
	int getWidth() const;
	int getHeight() const;
	void setWidthHeight(int width, int height);

public:
	bool start();

	bool update();
	void updateUniforms();

	bool processInput();
	void reloadShaders();
};
