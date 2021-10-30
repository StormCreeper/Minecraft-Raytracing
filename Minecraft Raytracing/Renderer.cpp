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
void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* msg, const void* data)
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
		std::cout << "Program failed! " << endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

	window_ = glfwCreateWindow(WIDTH, HEIGHT, "Minecraft Raytracing", nullptr, nullptr);
	if (!window_) {
		glfwTerminate();
		return true;
	}

	glfwMakeContextCurrent(window_);
	glewInit();

	glfwSwapInterval(0);

	//SETUP IMGUI
	ImguiWindowsManager::ImguiInit(window_);

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

	const std::vector<std::string> faces{
	   "skybox/right.jpg",
	   "skybox/left.jpg",
	   "skybox/top.jpg",
	   "skybox/bottom.jpg",
	   "skybox/front.jpg",
	   "skybox/back.jpg"
	};
	cube_texture = loadCubemap(faces);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture);

	glActiveTexture(GL_TEXTURE0);

	// 3D VOXEL TEXTURE

	vt.init();
	miniVt.init();

	double start = glfwGetTime();
	vt.generateTextureComputed();
	double end = glfwGetTime();

	miniVt.generateMiniTexture();
		
	std::cout << "Texture generation time : " << (end-start)*1000 << " ms\n";

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, vt.texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, miniVt.texture_id);

	// SETTING CALLBACKS

	glfwSetCursorPosCallback(window_, mousePosCallback);
	glfwSetWindowSizeCallback(window_, windowResizeCallback);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl_debug_callback, nullptr);

	glUseProgram(shader);

	while (!glfwWindowShouldClose(window_)) {
		if (!update())break;
	}

	ImguiWindowsManager::ImguiCleanup();

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shader);
	glDeleteTextures(1, &cube_texture);
	free(shader_error);

	return false;
}

bool Renderer::update() {

	glfwPollEvents();

	ImguiWindowsManager::ImguiCreateWindows(*this);

	camera.updateInput(window_, dt, b_paused);

	if (processInput()) return false;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, vt.texture_id);
	glUseProgram(shader);

	updateUniforms();

	//UPDATE dt
	const auto now = std::chrono::high_resolution_clock::now();
	dt = std::chrono::duration<float>(now - last_time).count();
	last_time = now;

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	ImguiWindowsManager::ImguiRender();

	glfwSwapBuffers(window_);

	return true;
}

void Renderer::updateUniforms() {
	camera.setUniforms(shader, true);
	camera.updateModel(shader);

	setUniformVec2(shader, "uResolution", static_cast<float>(WIDTH), static_cast<float>(HEIGHT));

	setUniformFloat(shader, "uTime", static_cast<float>(glfwGetTime()));

	setUniformVec3(shader, "mainMap.size", static_cast<float>(vt.dim[0]), static_cast<float>(vt.dim[1]), static_cast<float>(vt.dim[2]));
	setUniformInt(shader, "mainMap.tex", 0);

	setUniformVec3(shader, "miniMap.size", 128, 128, 128);
	setUniformInt(shader, "miniMap.tex", 2);

	setUniformInt(shader, "skybox", 1);

	setUniformFloat(shader, "wt.intensity", wt.intensity);
	setUniformVec2(shader, "wt.speed", wt.speed[0], wt.speed[1]);
	setUniformFloat(shader, "wt.diffuse", wt.diffuse);
	setUniformFloat(shader, "wt.reflection", wt.reflection);
	setUniformFloat(shader, "wt.refraction", wt.refraction);
	setUniformFloat(shader, "wt.ior", wt.ior);

	setUniformFloat(shader, "air_absorbance", air_absorbance);
	setUniformFloat(shader, "water_absorbance", water_absorbance);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, vt.texture_id);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, miniVt.texture_id);
}

bool enter_unpressed = true;
bool escape_unpressed = true;
bool m_unpressed = true;

bool Renderer::processInput() {
	if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (escape_unpressed) {
			if (b_paused) {
				glfwSetCursorPos(window_, 200, 200);
			}
			b_paused = !b_paused;

			escape_unpressed = false;
		}
	} else {
		escape_unpressed = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (enter_unpressed) {
			reloadShaders();
			enter_unpressed = false;
		}
	} else {
		enter_unpressed = true;
	}
	if (glfwGetKey(window_, GLFW_KEY_P) == GLFW_PRESS) {
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
	glCheckError();
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
	}
}