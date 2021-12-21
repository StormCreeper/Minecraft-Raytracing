#pragma once
#define GLEW_STATIC
#include "utils.h"
#include <chrono>
#include "Camera.h"
#include "VoxelTexture.h"
#include "Player.h"
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
	GLFWwindow* window;
public:
	float dt;
	std::chrono::steady_clock::time_point last_time;

	char title[33] = "Raytraced Minecraft - 0000 FPS";
public:
	bool b_paused;

	//TEXTURES

	VoxelTexture vt;
	VoxelTexture miniVt;

	// OBJECTS

	unsigned int shader;
	unsigned int vbo, vao;

	Player player;

	char* shader_error;

	bool drawWindows = true;
	bool rcp;
	bool rfp;

	int blockScale = 64;

	WaterParameters wt = {
		0.1, {0.5, 0.3}, 1, 0.8, 0.7, 1.3
	};

	float air_absorbance = 3.25;
	float water_absorbance = 1.1;

	glm::vec3 color_green = glm::vec3(0.3, 1, 0.1);
	glm::vec3 color_brown = glm::vec3(0.8, 0.7, 0.2);

public:
	Renderer() = default;

	Renderer(const int w, const int h) {
		WIDTH = w;
		HEIGHT = h;
		last_time = std::chrono::high_resolution_clock::now();
	}

public: // GETTERS AND SETTERS
	Player* getPlayerPtr();
	int getWidth() const;
	int getHeight() const;
	void setWidthHeight(int width, int height);
	void mouseInput(int button, int action, int mods);
	void scrollInput(double xoffset, double yoffset);

public:
	bool start();

	bool update();
	void updateUniforms();

	bool processInput();
	void reloadShaders();
};