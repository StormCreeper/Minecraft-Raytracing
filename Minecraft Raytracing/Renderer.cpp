#include "Renderer.h"
#include <iostream>
#include "ImguiWindows.h"

Renderer* renderer_ptr;

// CALLBACKS

void mousePosCallback(GLFWwindow* window, const double position_x, const double position_y) {
	renderer_ptr->getCameraPtr()->mouseCallback(window, static_cast<float>(position_x), static_cast<float>(position_y), renderer_ptr->b_paused);
}
void windowResizeCallback(GLFWwindow* window, const int width, const int height) {
	glViewport(0, 0, width, height);
	renderer_ptr->setWidthHeight(width, height);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	renderer_ptr->mouseInput(button, action, mods);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	renderer_ptr->scrollInput(xoffset, yoffset);
}

void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
	char* _source;
	char* _type;
	char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = (char*)"API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = (char*)"WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = (char*)"SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = (char*)"THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = (char*)"APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = (char*)"UNKNOWN";
		break;

	default:
		_source = (char*)"UNKNOWN";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		_type = (char*)"ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = (char*)"DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = (char*)"UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = (char*)"PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = (char*)"PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = (char*)"OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = (char*)"MARKER";
		break;

	default:
		_type = (char*)"UNKNOWN";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = (char*)"HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = (char*)"MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = (char*)"LOW";
		//	return;
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = (char*)"NOTIFICATION";
		return;
		break;

	default: 
		_severity = (char*)"UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n",
		id, _type, _severity, _source, msg);
}

bool Renderer::start() {
	renderer_ptr = this;

	if (!glfwInit()) {
		std::cout << "Program failed! " << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Minecraft Raytracing", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return true;
	}

	glfwMakeContextCurrent(window);
	glewInit();

	glfwSwapInterval(0);

	camera.resize(WIDTH, HEIGHT);

	//SETUP IMGUI
	ImguiWindowsManager::ImguiInit(window);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glfwSwapInterval(0);

	// SCREEN QUAD CREATION

	float vertices[] = {
		 1,  1,
		 1, -1,
		-1, -1,
		-1,  1,
		 1,  1,
		-1, -1
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	shader = createProgram("shader.vert", "shader.frag");
	glUseProgram(0);

	shader_error = static_cast<char*>(malloc(2048 * sizeof(char)));

	glActiveTexture(GL_TEXTURE0);

	// 3D VOXEL TEXTURE

	vt.init(256, 256, 256);
	miniVt.init(128, 128, 128);

	double start = glfwGetTime();
	vt.generateTextureComputed();
	double end = glfwGetTime();

	miniVt.LoadFromVoxFile("Vox/teapot.vox");
	//miniVt.generateMiniTexture();
	//miniVt.generateTextureComputed();

	std::cout << "Texture generation time : " << (end-start)*1000 << " ms\n";

	// SETTING CALLBACKS

	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, scroll_callback);


	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl_debug_callback, nullptr);

	glUseProgram(shader);

	while (!glfwWindowShouldClose(window)) {
		if (!update())
			break;
	}

	ImguiWindowsManager::ImguiCleanup();

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shader);
	free(shader_error);

	return false;
}

bool Renderer::update() {

	glfwPollEvents();

	ImguiWindowsManager::ImguiCreateWindows(*this);

	camera.updateInput(window, dt, b_paused);

	if (processInput()) return false;
	glUseProgram(shader);

	updateUniforms();

	//UPDATE dt
	const auto now = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float>(now - last_time).count();
	last_time = now;

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	ImguiWindowsManager::ImguiRender();

	glfwSwapBuffers(window);

	return true;
}

void Renderer::updateUniforms() {
	camera.setMatrices(shader);

	setUniformM4(shader, "u_InverseView", glm::inverse(camera.view));
	setUniformM4(shader, "u_InverseProjection", glm::inverse(camera.projection));

	setUniformVec2(shader, "u_Resolution", static_cast<float>(WIDTH), static_cast<float>(HEIGHT));

	setUniformFloat(shader, "u_Time", static_cast<float>(glfwGetTime()));

	setUniformVec3(shader, "u_MainMap.size", static_cast<float>(vt.dim[0]), static_cast<float>(vt.dim[1]), static_cast<float>(vt.dim[2]));
	setUniformInt(shader, "u_MainMap.tex", 0);

	setUniformVec3(shader, "u_MiniMap.size", float(blockScale), float(blockScale), float(blockScale));
	setUniformInt(shader, "u_MiniMap.tex", 1);

	setUniformFloat(shader, "u_MiniVoxResolution", float(blockScale));

	setUniformFloat(shader, "u_WaterParams.intensity", wt.intensity);
	setUniformVec2 (shader, "u_WaterParams.speed", wt.speed[0], wt.speed[1]);
	setUniformFloat(shader, "u_WaterParams.diffuse", wt.diffuse);
	setUniformFloat(shader, "u_WaterParams.reflection", wt.reflection);
	setUniformFloat(shader, "u_WaterParams.refraction", wt.refraction);
	setUniformFloat(shader, "u_WaterParams.ior", wt.ior);

	setUniformFloat(shader, "air_absorbance", air_absorbance);
	setUniformFloat(shader, "water_absorbance", water_absorbance);

	for (int i = 0; i < 256; i++) {
		std::string s = "palette[";
		s += std::to_string(i);
		s += "]";
		setUniformVec3(shader, s.c_str(), miniVt.palette[i]);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, vt.texture_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, miniVt.texture_id);

	unsigned char block;
	vec3 normal;

	int x, y, z;

	float d = voxel_traversal(vt, camera.position, camera.front, normal, block, x, y, z);

	if (d >= 0)
		setUniformVec3i(shader, "selected", x, y, z);
	else
		setUniformVec3i(shader, "selected", -1, -1, -1);

}

bool enter_unpressed = true;
bool escape_unpressed = true;
bool m_unpressed = true;

bool Renderer::processInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (escape_unpressed) {
			if (b_paused) {
				glfwSetCursorPos(window, 200, 200);
			}
			b_paused = !b_paused;

			escape_unpressed = false;
		}
	} else {
		escape_unpressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (enter_unpressed) {
			reloadShaders();
			enter_unpressed = false;
		}
	} else {
		enter_unpressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		if (m_unpressed) {
			drawWindows = !drawWindows;
			m_unpressed = false;
		}
	} else {
		m_unpressed = true;
	}

	return false;
}

void Renderer::reloadShaders() {
	memset(shader_error, 0, 2048 * sizeof(char));

	const unsigned int new_shader = createProgram("vert.glsl", "frag.glsl", &shader_error);

	if (new_shader != 128) {
		glDeleteProgram(shader);
		shader = new_shader;
		glUseProgram(new_shader);
	}


	glUseProgram(shader);
}

// GETTERS AND SETTERS

Camera* Renderer::getCameraPtr() {
	return &camera;
}

int Renderer::getWidth() const {
	return WIDTH;
}

int Renderer::getHeight() const {
	return HEIGHT;
}

void Renderer::setWidthHeight(const int width, const int height) {
	if (width * height == 0 || width < 0 || height < 0) {
		std::cout << "Error : Wrong screen dimensions";
	}
	else {
		WIDTH = width;
		HEIGHT = height;

		camera.resize(width, height);
	}
}

uint8_t queryVoxel(VoxelTexture& tex, int x, int y, int z) {
	if (x < 0 || y < 0 || z < 0) return 0;
	if (x >= tex.dim[0] || y >= tex.dim[1] || z >= tex.dim[2]) return 0;

	return tex.runTimeData[index(x, y, z, tex.dim[0], tex.dim[1], tex.dim[2])];
}

void setVoxel(VoxelTexture& tex, int x, int y, int z, uint8_t vox) {
	if (x < 0 || y < 0 || z < 0) return;
	if (x >= tex.dim[0] || y >= tex.dim[1] || z >= tex.dim[2]) return;

	GLuint data[1] = { vox };
	glBindTexture(GL_TEXTURE_3D, tex.texture_id);

	glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
	tex.runTimeData[index(x, y, z, tex.dim[0], tex.dim[1], tex.dim[2])] = vox;
}

float voxel_traversal(VoxelTexture &tex, vec3 origin, vec3 direction, vec3 &normal, uint8_t &block, int &mapX, int &mapY, int &mapZ) {

	mapX = int(floor(origin.x));
	mapY = int(floor(origin.y));
	mapZ = int(floor(origin.z));

	block = queryVoxel(tex, mapX, mapY, mapZ);
	if (block != 0) {
		normal = vec3(0, 0, 0);
		return 0;
	}

	float sideDistX;
	float sideDistY;
	float sideDistZ;

	float deltaDX = abs(1 / direction.x);
	float deltaDY = abs(1 / direction.y);
	float deltaDZ = abs(1 / direction.z);
	float perpWallDist = -1;

	int stepX;
	int stepY;
	int stepZ;

	int hit = 0;
	int side;

	if (direction.x < 0) {
		stepX = -1;
		sideDistX = (origin.x - mapX) * deltaDX;
	}
	else {
		stepX = 1;
		sideDistX = (float(mapX) + 1.0 - origin.x) * deltaDX;
	}
	if (direction.y < 0) {
		stepY = -1;
		sideDistY = (origin.y - mapY) * deltaDY;
	}
	else {
		stepY = 1;
		sideDistY = (float(mapY) + 1.0 - origin.y) * deltaDY;
	}
	if (direction.z < 0) {
		stepZ = -1;
		sideDistZ = (origin.z - mapZ) * deltaDZ;
	}
	else {
		stepZ = 1;
		sideDistZ = (float(mapZ) + 1.0 - origin.z) * deltaDZ;
	}

	int step = 1;

	for (int i = 0; i < 15; i++) {
		if ((mapX >= tex.dim[0] && stepX > 0) || (mapY >= tex.dim[1] && stepY > 0) || (mapZ >= tex.dim[2] && stepZ > 0)) break;
		if ((mapX < 0 && stepX < 0) || (mapY < 0 && stepY < 0) || (mapZ < 0 && stepZ < 0)) break;

		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDX;
			mapX += stepX * step;
			side = 0;
		}
		else if (sideDistY < sideDistX && sideDistY < sideDistZ) {
			sideDistY += deltaDY;
			mapY += stepY * step;
			side = 1;
		}
		else {
			sideDistZ += deltaDZ;
			mapZ += stepZ * step;
			side = 2;
		}

		block = queryVoxel(tex, mapX, mapY ,mapZ);
		if (block != 0 && block != 14) {
			if (side == 0) {
				perpWallDist = (mapX - origin.x + (1 - stepX * step) / 2) / direction.x;
				normal = vec3(1, 0, 0) * -float(stepX);
			}
			else if (side == 1) {
				perpWallDist = (mapY - origin.y + (1 - stepY * step) / 2) / direction.y;
				normal = vec3(0, 1, 0) * -float(stepY);
			}
			else {
				perpWallDist = (mapZ - origin.z + (1 - stepZ * step) / 2) / direction.z;
				normal = vec3(0, 0, 1) * -float(stepZ);
			}

			break;
		}
	}

	return perpWallDist;
}

void Renderer::mouseInput(int button, int action, int mods) {
	if (b_paused) return;

	if (button == 0 && action == GLFW_PRESS) {
		uint8_t block;
		vec3 normal;

		int x, y, z;

		float d = voxel_traversal(vt, camera.position, camera.front, normal, block, x, y, z);

		if (d >= 0) {
			setVoxel(vt, x, y, z, 0);
		}
	}

	if (button == 1 && action == GLFW_PRESS) {
		uint8_t block;
		vec3 normal;

		int x, y, z;

		float d = voxel_traversal(vt, camera.position, camera.front, normal, block, x, y, z);

		if (d >= 0) {
			vec3 pos = camera.position + camera.front * d + normal;

			x += int(normal.x);
			y += int(normal.y);
			z += int(normal.z);

			setVoxel(vt, x, y, z, tool);
		}
	}
	
	if (button == 2 && action == GLFW_PRESS) {
		uint8_t block;
		vec3 normal;

		int x, y, z;

		float d = voxel_traversal(vt, camera.position, camera.front, normal, block, x, y, z);

		if (d >= 0) {
			tool = block;
		}
	}
}

void Renderer::scrollInput(double xoffset, double yoffset) {
	tool = (tool - int(yoffset) + 15 - 1) % 15 + 1;
}
