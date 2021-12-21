#include "Camera.h"

constexpr float degtorad = 3.1415926535f / 180.0f;


Camera::Camera() : speed(14.0f), last_x(0), last_y(0), projection(), view(), height(), width() {
	position = vec3(-5, 3, -5);
	rotation_x = 0;
	rotation_y = 0;

	front = vec3(0, 0, 1);
	up = vec3(0, 1, 0);
	world_up = vec3(0, 1, 0);
	right = vec3(1, 0, 0);

	first_mouse = true;
}
Camera::Camera(float aspect, const glm::vec3 pos) : speed(14.0f), last_x(0), last_y(0), projection(), view(), height(), width() {
	position = pos;
	rotation_x = 0;
	rotation_y = 0;

	front = glm::vec3(0, 0, 1);
	up = glm::vec3(0, 1, 0);
	world_up = glm::vec3(0, 1, 0);
	right = glm::vec3(1, 0, 0);

	first_mouse = true;
}

void Camera::setMatrices(const unsigned int shader, float fov) {
	projection = glm::perspective(glm::radians<float>(fov), float(width) / float(height), 0.01f, 1200.0f);
	view = glm::lookAt(position, position + front, up);
}

void Camera::mouseCallback(GLFWwindow* window, const float position_x, const float position_y) {
	if (first_mouse) {
		last_x = position_x;
		last_y = position_y;
		first_mouse = false;
	}

	const float offset_x = position_x - last_x;
	const float offset_y = position_y - last_y;
	last_x = width/2;
	last_y = height/2;

	const float sensitivity = 0.2f;

	rotation_y += offset_x * sensitivity;
	rotation_x -= offset_y * sensitivity;

	if (rotation_x > 89.0f)
		rotation_x = 89.0f;
	if (rotation_x < -89.0f)
		rotation_x = -89.0f;

	glm::vec3 direction;
	direction.x = cos(rotation_y * degtorad) * cos(rotation_x * degtorad);
	direction.y = sin(rotation_x * degtorad);
	direction.z = sin(rotation_y * degtorad) * cos(rotation_x * degtorad);
	front = glm::normalize(direction);
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));

	glfwSetCursorPos(window, width / 2, height / 2);
	
}

void Camera::resize(int width, int height) {
	projection = glm::perspective(glm::radians<float>(80), float(width) / float(height), 0.01f, 1200.0f);
	this->width = width;
	this->height = height;
}

glm::vec3 Camera::getPos() const {
	return position;
}