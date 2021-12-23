#pragma once
#include "utils.h"
#include "Camera.h"
#include "PhysicsSolver.h"

class Player {
public:
	Player();

	Camera camera;
	glm::vec3 position;
	glm::vec3 velocity;

	float speed = 7;
	int speed_mod = 0;

	bool touchingGround = false;
	bool speeding = false;
	bool flying = false;
	bool swimming = false;

	float fov = 70.0f;

	bool spaceReleased = true;
	float lastTimeSpaceBarPressed = 0;

	ivec3 selectedBlock;

	VoxelTexture* vt;

	void KeyInput(GLFWwindow* window, float deltaTime);
	void MouseMoveInput(GLFWwindow* window, float position_x, float position_y);
	void PhysicsUpdate(float deltaTime);
	void MouseButtonInput(int button, int action, int mods);

	bool Colliding(std::vector<AABB>& aabbs);
};