#pragma once
#include "utils.h"
#include <GLFW/glfw3.h>

typedef class Camera Camera;
class Camera {
public:
	float rotation_x;
	float rotation_y;
	float speed;
	float last_x;
	float last_y;
	bool first_mouse;

	int width, height;

	mat4 projection;
	mat4 view;

	int speed_mod;

	Camera();
	Camera(float aspect, vec3 pos);

	void setMatrices(unsigned int shader);

	void mouseCallback(GLFWwindow* window, float position_x, float position_y);
	void resize(int width, int height);

	vec3 getPos() const;

	vec3 position;
	vec3 velocity;
	vec3 front;
	vec3 up;
	vec3 right;
	vec3 world_up;
};