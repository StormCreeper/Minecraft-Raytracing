#include "Camera.h"

constexpr float degtorad = 3.1415926535f / 180.0f;


Camera::Camera() : speed(14.0f), speed_mod(0), last_x(0), last_y(0), projection(), view(), height(), width() {
	position = vec3(-5, 3, -5);
	velocity = vec3(0, 0, 0);
	rotation_x = 0;
	rotation_y = 0;

	front = vec3(0, 0, 1);
	up = vec3(0, 1, 0);
	world_up = vec3(0, 1, 0);
	right = vec3(1, 0, 0);

	first_mouse = true;
}
Camera::Camera(float aspect, const glm::vec3 pos) : speed(14.0f), speed_mod(0), last_x(0), last_y(0), projection(), view(), height(), width() {
	position = pos;
	velocity = glm::vec3(0, 0, 0);
	rotation_x = 0;
	rotation_y = 0;

	front = glm::vec3(0, 0, 1);
	up = glm::vec3(0, 1, 0);
	world_up = glm::vec3(0, 1, 0);
	right = glm::vec3(1, 0, 0);

	first_mouse = true;
}

void Camera::setMatrices(const unsigned int shader) {
	view = glm::lookAt(position, position + front, up);
}


void Camera::updateInput(GLFWwindow* window, const float delta_time, const bool b_paused) {
	if (b_paused) return;
	float acc = 1 * powf(2, speed_mod);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) acc *= 20.0f;

	glm::vec3 tmp_front = glm::normalize(glm::vec3(front.x, 0, front.z));
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) velocity += tmp_front * acc * speed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) velocity -= tmp_front * acc * speed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) velocity -= right * acc * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) velocity += right * acc * speed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) velocity += world_up * acc * speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) velocity -= world_up * acc * speed;

	position += velocity * delta_time;
	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;
}
void Camera::mouseCallback(GLFWwindow* window, const float position_x, const float position_y, const bool b_paused) {
	if (b_paused) return;
	
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