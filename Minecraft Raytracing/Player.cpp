#include "Player.h"
#include <iostream>f

Player::Player() : camera(1, glm::vec3(-10, 100, -10)), position(-10, 100, -10), velocity(0, 0, 0) {

}

void Player::KeyInput(GLFWwindow* window) {
	velocity = glm::vec3(0, 0, 0);

	glm::vec3 tmp_front = glm::normalize(glm::vec3(camera.front.x, 0, camera.front.z));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)            velocity += tmp_front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)            velocity -= tmp_front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)            velocity -= camera.right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)            velocity += camera.right;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)        velocity += camera.world_up;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) velocity -= camera.world_up;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) velocity *= 10;
}

void Player::MouseMoveInput(GLFWwindow* window, float position_x, float position_y) {
	camera.mouseCallback(window, position_x, position_y);

	unsigned char block;
	vec3 normal;

	float d = voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
	if (d < 0) selectedBlock = ivec3(-1, -1, -1);
}

void Player::PhysicsUpdate(float deltaTime) {
	position += velocity * speed * deltaTime;
	camera.position = position;
}

void Player::MouseButtonInput(int button, int action, int mods) {
	uint8_t block;
	vec3 normal;

	float d = voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
	if (d < 0) selectedBlock = ivec3(-1, -1, -1);

	if (d >= 0) {

		if (button == 0 && action == GLFW_PRESS)
			setVoxel(*vt, selectedBlock.x, selectedBlock.y, selectedBlock.z, 0);

		if (button == 1 && action == GLFW_PRESS)
				setVoxel(*vt, selectedBlock.x + int(normal.x), selectedBlock.y + int(normal.y), selectedBlock.z + int(normal.z), tool);

		if (button == 2 && action == GLFW_PRESS)
				tool = block;
	}

	voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
}
