#pragma once
#include "utils.h"
#include "Camera.h"

class Player {
public:
	Player();

	Camera camera;
	glm::vec3 position;
	glm::vec3 velocity;

	float speed = 14;

	int tool = 15;
	ivec3 selectedBlock;

	VoxelTexture* vt;

	void KeyInput(GLFWwindow* window);
	void MouseMoveInput(GLFWwindow* window, float position_x, float position_y);
	void PhysicsUpdate(float deltaTime);
	void MouseButtonInput(int button, int action, int mods);
};