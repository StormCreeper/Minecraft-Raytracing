#pragma once
#define GLEW_STATIC
#include "utils.h"
#include <chrono>
#include "Camera.h"
#include "VoxelTexture.h"
#include <GLFW/glfw3.h>

struct WaterParameters {
	float intensity;
	float speed[2];
	float diffuse;
	float reflection;
	float refraction;
	float ior;
};

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

	bool drawWindows = true;
	bool rcp;
	bool rfp;

	WaterParameters wt = {
		1, {5, 4}, 1, 0.8, 0.7, 1.3
	};

	float air_absorbance = 3.25;
	float water_absorbance = 1.3;

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
