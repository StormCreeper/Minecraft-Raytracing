#include "Renderer.h"
#include <iostream>
#include "ImguiWindows.h"

Renderer* renderer_ptr;

// CALLBACKS

void mousePosCallback(GLFWwindow* window, const double position_x, const double position_y) {
	if (!renderer_ptr->b_paused)
		renderer_ptr->getPlayerPtr()->MouseMoveInput(window, static_cast<float>(position_x), static_cast<float>(position_y));
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

	player.camera.resize(WIDTH, HEIGHT);
	player.vt = &vt;

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

	//UPDATE dt

	const auto now = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float>(now - last_time).count();
	last_time = now;

	ImguiWindowsManager::ImguiCreateWindows(*this);

	// Manage events

	glfwPollEvents();

	if (!b_paused)
		player.KeyInput(window, dt);

	if (processInput()) return false;

	if(!b_paused)
		player.PhysicsUpdate(dt);

	glUseProgram(shader);
	
	updateUniforms();

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	ImguiWindowsManager::ImguiRender();

	glfwSwapBuffers(window);

	return true;
}

void Renderer::updateUniforms() {
	player.camera.setMatrices(shader, player.fov);

	setUniformM4(shader, "u_InverseView", glm::inverse(player.camera.view));
	setUniformM4(shader, "u_InverseProjection", glm::inverse(player.camera.projection));

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

	setUniformVec3(shader, "colorGreen", color_green);
	setUniformVec3(shader, "colorBrown", color_brown);

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
	
	ivec3 selected = player.selectedBlock;
	setUniformVec3i(shader, "selected", selected.x, selected.y, selected.z);

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

Player* Renderer::getPlayerPtr() {
	return &player;
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

		player.camera.resize(width, height);
	}
}

void Renderer::mouseInput(int button, int action, int mods) {
	if (b_paused) return;
	player.MouseButtonInput(button, action, mods);
	
}

void Renderer::scrollInput(double xoffset, double yoffset) {
	player.tool = (player.tool - int(yoffset) + 15 - 1) % 15 + 1;
}
