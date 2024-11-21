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
	std::chrono::system_clock::time_point last_time;

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