#include "VoxelTexture.h"
#include "utils.h"
#include <iostream>
#include <GLFW/glfw3.h>

VoxelTexture::VoxelTexture() {
	dim[0] = 64;
	dim[1] = 64;
	dim[2] = 64;

	ldim[0] = 64;
	ldim[1] = 64;
	ldim[2] = 64;

	texture_id = 0;
	shader_id = 0;
}

void VoxelTexture::init() {

	if (shader_id) glDeleteProgram(shader_id);

	if (!texture_id) {
		glGenTextures(1, &texture_id);

		glBindTexture(GL_TEXTURE_3D, texture_id);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dim[0], dim[1], dim[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
	}
	char* shaderSource = readSource("compute.glsl");
	unsigned int compute_shader = createShader(GL_COMPUTE_SHADER, shaderSource);

	free(shaderSource);

	if (compute_shader == 128) return;

	shader_id = glCreateProgram();

	glAttachShader(shader_id, compute_shader);
	glLinkProgram(shader_id);
	glDeleteShader(compute_shader);
}

void VoxelTexture::generateTextureComputed() {

	glCheckError();

	if (ldim[0] != dim[0] || dim[1] != ldim[1] || ldim[2] != dim[2]) {
		glBindTexture(GL_TEXTURE_3D, texture_id);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dim[0], dim[1], dim[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
		for (int i = 0; i < 3; i++) ldim[i] = dim[i];
	}

	glUseProgram(shader_id);

	setUniformFloat(shader_id, "iTime", glfwGetTime());

	glBindTexture(GL_TEXTURE_3D, texture_id);
	glBindImageTexture(0, texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glDispatchCompute(dim[0]/16, dim[1]/16, dim[2]/4);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glBindTexture(GL_TEXTURE_3D, 0);

	glUseProgram(0);

	glCheckError();
}

VoxelTexture::~VoxelTexture() {
	glDeleteTextures(1, &texture_id);
	glDeleteProgram(shader_id);

	if(data != nullptr)
		free(data);
}