#include "Player.h"
#include <iostream>
#include "SaveManager.h"
#include "SettingsManager.h"

Player::Player() : camera(1, glm::vec3(-0,0,0)), position(100, 100, 100), velocity(0, 0, 0), selectedBlock(0), vt(nullptr) {

}

void Player::KeyInput(GLFWwindow* window, float deltaTime) {
	velocity.x = 0;
	velocity.z = 0;

	glm::vec3 tmp_front = glm::normalize(glm::vec3(camera.front.x, 0, camera.front.z));

	speeding = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)            velocity += tmp_front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)            velocity -= tmp_front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)            velocity -= camera.right;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)            velocity += camera.right;
	
	glm::vec3 up = camera.world_up;
	float upvel = 0.5;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if(flying)
			velocity += up * upvel;
		if (touchingGround && velocity.y <= 0.1) {
			velocity = up * 1.8f;
		}
		if (spaceReleased && glfwGetTime() - lastTimeSpaceBarPressed < 0.1f) {
			flying = !flying;
		}
		if(swimming)
			velocity += up * 0.05f;

		spaceReleased = false;
		lastTimeSpaceBarPressed = glfwGetTime();
	} else {
		spaceReleased = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		if(flying)
			velocity -= up * upvel;
		if(swimming)
			velocity -= up * 0.05f;

	}
	if (speeding)
		fov += (110 - fov) * 5 * deltaTime;
	else
		fov += (80 - fov) * 5 * deltaTime;
}

void Player::MouseMoveInput(GLFWwindow* window, float position_x, float position_y) {
	camera.mouseCallback(window, position_x, position_y);

	unsigned char block;
	vec3 normal;

	float d = voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
	if (d < 0) selectedBlock = ivec3(-1, -1, -1);
}

bool Player::Colliding(std::vector<AABB> &aabbs) {
	AABB playerBB = {
		position - glm::vec3(0.25, 1.5, 0.25),
		glm::vec3(0.5, 2, 0.5)
	};

	for (AABB& aabb : aabbs) {
		if (collide(playerBB, aabb)) {
			return true;
		}
	}

	return false;
}

void Player::PhysicsUpdate(float deltaTime) {

	std::vector<AABB> aabbs;

	float volume = 0;

	AABB playerBB = {
		position - glm::vec3(0.25, 1.5, 0.25),
		glm::vec3(0.5, 1.7, 0.5)
	};

	glm::vec3 instantVel = velocity * speed * powf(2.0f, speed_mod) * deltaTime;

	float minx = int(std::min(position.x, position.x + instantVel.x));
	float maxx = int(std::max(position.x, position.x + instantVel.x));
	float miny = int(std::min(position.y, position.y + instantVel.y));
	float maxy = int(std::max(position.y, position.y + instantVel.y));
	float minz = int(std::min(position.z, position.z + instantVel.z));
	float maxz = int(std::max(position.z, position.z + instantVel.z));

	for (int x = minx - 1; x <= maxx + 1; x++)
		for (int y = miny - 2; y <= maxy + 2; y++)
			for (int z = minz - 1; z <= maxz + 1; z++) {
				uint8_t block = queryVoxel(*vt, x, y, z);
				if (block != 0 && block != 14) aabbs.push_back(
					{ glm::vec3(x, y, z),
					glm::vec3(1, 1, 1) }
				);
				if (block == 14) {
					AABB inter;
					intersection({ glm::vec3(x, y, z), glm::vec3(1, 1, 1) }, playerBB, inter);
					glm::vec3 dv = max(glm::vec3(0, 0, 0), inter.dim);

					volume += dv.x * dv.y * dv.z;
				}
			}


	swimming = volume > 0;
	position.y -= 0.03;
	touchingGround = Colliding(aabbs);
	position.y += 0.03;

	if (touchingGround) {
		flying = false;
		//if (velocity.y < 0) velocity.y = 0;
	}
	if (speeding) {
		if(flying)
			instantVel *= glm::vec3(5, 5, 5);
		else 
			instantVel *= glm::vec3(1.5, 1, 1.5);
	}
	if (flying) instantVel *= glm::vec3(1.5, 1, 1.5);
	
	position.y += instantVel.y;
	
	if (Colliding(aabbs)) {
		position.y -= instantVel.y * 1.01f;
		velocity.y *= -0.05;
	}

	position.y -= 0.03;
	touchingGround = Colliding(aabbs);
	position.y += 0.03;

	position.x += instantVel.x;
	if (Colliding(aabbs)) position.x -= instantVel.x * 1.01f;

	position.z += instantVel.z;
	if (Colliding(aabbs)) position.z -= instantVel.z * 1.01f;

	glm::vec3 g = glm::vec3(0, -9.81, 0);

	if (!flying) {
		float mass = (playerBB.dim.x * playerBB.dim.y * playerBB.dim.z);
		if (mass < 0.1) mass = 0.1;

		glm::vec3 forces = glm::vec3(0, 0, 0);

		float vel = length(velocity);
		if(!touchingGround)
			forces += g * mass;

		float ro = 0.9;
		forces -= g * ro * volume;

		float a = 5 * volume / mass;
		glm::vec3 nvel = velocity;

		if (glm::length(nvel) > 0) nvel = glm::normalize(nvel);
		forces -= a * pow(vel, 1.4f) * nvel;

		velocity += forces / mass * deltaTime;
	}
	else
		velocity.y *= 0.8;

	camera.position = position;
}

void Player::MouseButtonInput(int button, int action, int mods) {
	uint8_t block;
	vec3 normal;

	float d = voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
	if (d < 0) selectedBlock = ivec3(-1, -1, -1);

	if (d >= 0) {

		if (button == 0 && action == GLFW_PRESS) {
			setVoxel(*vt, selectedBlock.x, selectedBlock.y, selectedBlock.z, 0);
			SaveManager::addAction(selectedBlock, 0);
		}

		if (button == 1 && action == GLFW_PRESS) {
			glm::ivec3 pos = glm::vec3(selectedBlock.x + int(normal.x), selectedBlock.y + int(normal.y), selectedBlock.z + int(normal.z));
			
			setVoxel(*vt, pos.x, pos.y, pos.z, SettingsManager::get<int>("tool"));
			SaveManager::addAction(pos, block);
		}
		if (button == 2 && action == GLFW_PRESS)
			SettingsManager::set<int>("tool", block);
	}

	voxel_traversal(*vt, position, camera.front, normal, block, selectedBlock.x, selectedBlock.y, selectedBlock.z);
}
