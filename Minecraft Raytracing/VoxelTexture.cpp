#include "VoxelTexture.h"
#include "utils.h"
#include <random>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <GLFW/glfw3.h>

unsigned int VoxelTexture::generateTextureComputed() {

	if(id) glDeleteTextures(1, &id);
	w = 256;
	h = 256;
	d = 256;

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_3D, id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, w, h, d, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);


	char* shaderSource = readSource("compute.glsl");
	unsigned int compute_shader = createShader(GL_COMPUTE_SHADER, shaderSource);

	free(shaderSource);

	if(compute_shader == 128) return 0;

	unsigned int shader_program = glCreateProgram();

	glAttachShader(shader_program, compute_shader);
	glLinkProgram(shader_program);
	glDeleteShader(compute_shader);

	glUseProgram(shader_program);


	//setUniformFloat(shader_program, "iTime", glfwGetTime());

	glBindTexture(GL_TEXTURE_3D, id);
	glBindImageTexture(0, id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glDispatchCompute(w/8, h/8, d/8);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glBindTexture(GL_TEXTURE_3D, 0);
	glUseProgram(0);

	glCheckError();

	glDeleteProgram(shader_program);

	return 0;
}

VoxelTexture::~VoxelTexture() {
	glDeleteTextures(1, &id);

	if(data != nullptr)
		free(data);
}