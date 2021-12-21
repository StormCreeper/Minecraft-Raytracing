#pragma once

#include "utils.h"

struct AABB {
	glm::vec3 corner;
	glm::vec3 dim;
};

struct PhysicsObject {
	AABB bb;
	glm::vec3 velocity;
};

bool collide(const AABB& a, const AABB& b);
void intersection(const AABB& a, const AABB& b, AABB& c);