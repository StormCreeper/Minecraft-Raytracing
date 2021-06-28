#include "Camera.h"
Camera::Camera() : speed_(4.0f), speed_mod(0), last_x_(0), last_y_(0) {
	projection_ = mat44::CreateIdentity();
	view_ = mat44::CreateIdentity();
	position = v3(-5, 3, -5);
	velocity = v3(0, 0, 0);
	rotation_x_ = 0;
	rotation_y_ = 0;

	front = v3(0, 0, 1);
	up_ = v3(0, 1, 0);
	world_up_ = v3(0, 1, 0);
	right_ = v3(1, 0, 0);

	first_mouse_ = true;
}
Camera::Camera(float aspect, const v3 pos) : speed_(14.0f), speed_mod(0), last_x_(0), last_y_(0) {
	projection_ = mat44::CreateIdentity();
	view_ = mat44::CreateIdentity();
	position = pos;
	velocity = v3(0, 0, 0);
	rotation_x_ = 0;
	rotation_y_ = 0;

	front = v3(0, 0, 1);
	up_ = v3(0, 1, 0);
	world_up_ = v3(0, 1, 0);
	right_ = v3(1, 0, 0);

	first_mouse_ = true;
}

void Camera::setUniforms(unsigned int shader, const bool reset) {
	if (reset) {
		view_ = mat44::LookAt(position, position + front, up_);
	}
}
void Camera::updateModel(const unsigned int shader) const
{
	setUniformVec3(shader, "viewPos", position);
	setUniformM4(shader, "view", view_);
}

void Camera::updateInput(GLFWwindow* window, const float delta_time, const bool b_paused) {
	if (!b_paused) {
		float acc = 1 * powf(2, speed_mod);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) acc *= 20.0f;

		v3 tmp_front = v3(front.x, 0, front.z).norm();
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) velocity += tmp_front * acc * speed_;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) velocity -= tmp_front * acc * speed_;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) velocity -= right_ * acc * speed_;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) velocity += right_ * acc * speed_;
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) velocity += world_up_ * acc * speed_;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) velocity -= world_up_ * acc * speed_;

		position += velocity * delta_time;
		velocity.x = 0;
		velocity.y = 0;
		velocity.z = 0;
	}
}
void Camera::mouseCallback(GLFWwindow* window, const float position_x, const float position_y, const bool b_paused) {
	if (!b_paused) {
		if (first_mouse_) {
			last_x_ = position_x;
			last_y_ = position_y;
			first_mouse_ = false;
		}

		const float offset_x = position_x - last_x_;
		const float offset_y = last_y_ - position_y;
		last_x_ = 200;
		last_y_ = 200;

		const float sensitivity = 0.2f;

		rotation_y_ += offset_x * sensitivity;
		rotation_x_ += offset_y * sensitivity;

		if (rotation_x_ > 89.0f)
			rotation_x_ = 89.0f;
		if (rotation_x_ < -89.0f)
			rotation_x_ = -89.0f;

		v3 direction;
		direction.x = cos(rotation_y_ * degtorad) * cos(rotation_x_ * degtorad);
		direction.y = sin(rotation_x_ * degtorad);
		direction.z = sin(rotation_y_ * degtorad) * cos(rotation_x_ * degtorad);
		front = direction.norm();
		right_ = front.cross(world_up_).norm();
		up_ = right_.cross(front).norm();

		glfwSetCursorPos(window, 200, 200);
	}
}

v3 Camera::getPos() const {
	return position;
}