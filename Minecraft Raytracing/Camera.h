#pragma once
#include "utils.h"
#include <GLFW/glfw3.h>

typedef class Camera Camera;
class Camera {
private:
	mat44 projection_;
	mat44 view_;
	v3 up_;
	v3 right_;
	v3 world_up_;
	float rotation_x_;
	float rotation_y_;
	float speed_;
	float last_x_;
	float last_y_;
	bool first_mouse_;
public:
	int speed_mod;

	Camera();
	Camera(float aspect, v3 pos);

	void setUniforms(unsigned int shader, bool reset);
	void updateModel(unsigned int shader) const;
	void updateInput(GLFWwindow* window, float delta_time, bool b_paused);
	void mouseCallback(GLFWwindow* window, float position_x, float position_y, bool b_paused);

	v3 getPos() const;

	v3 position;
	v3 velocity;
	v3 front;
};