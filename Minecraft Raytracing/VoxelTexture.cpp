#include "VoxelTexture.h"
#include "utils.h"
#include <random>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <GLFW/glfw3.h>


float getHeight(int x, int z) {
	float height = 0;

	float amplitude = 0.25f;
	float frequency = 0.002f;

	height += SimplexNoise::noise(x * frequency, z * frequency) * amplitude;

	const float smoothness = height * 0.6 + 0.9;

	for (int i = 0; i < 6; i++) {
		height += SimplexNoise::noise(x * frequency, z * frequency) * amplitude;

		frequency *= 2;
		amplitude *= 0.5f * smoothness;
	}

	height = height * 128 + 128;
	return height;
}

unsigned int VoxelTexture::generateTerrainTexture() {
	w = 512;
	h = 256;
	d = 512;
	srand(static_cast<unsigned int>(time(nullptr)));
	data = static_cast<float*>(malloc(w * h * d * 4 * sizeof(float)));

	for (int x = 0; x < w; x++) {
		for (int z = 0; z < d; z++) {
			const float height = getHeight(x, z);
			const float tint = SimplexNoise::noise((x + 1000) * 0.01, z * 0.01) * 0.5 + 0.5 * 2;
			const float col = (rand() % 1000) / 5000.0;
			for (int y = 0; y < h; y++) {
				const float locVar = (rand() % 1000) / 1000.0;
				const float he = y / static_cast<float>(h);
				const int i = z * (w * h) + (y * w + x);
				if (y < height - 20) {
					data[i * 4 + 0] = 0.7 + col + locVar;
					data[i * 4 + 1] = 0.7 + col + locVar;
					data[i * 4 + 2] = 0.7 + col + locVar;
					data[i * 4 + 3] = 0;
				}
				else if (y < height - 5) {
					data[i * 4 + 0] = (0.7 + col + locVar) * 0.8;
					data[i * 4 + 1] = (0.7 + col + locVar) * 0.4;
					data[i * 4 + 2] = (0.7 + col + locVar) * 0.3;
					data[i * 4 + 3] = 0;
				}
				else if (y < height) {
					data[i * 4 + 0] = (0.7 + col + locVar) * he * tint;
					data[i * 4 + 1] = 0.7 + col + locVar;
					data[i * 4 + 2] = (0.7 + col + locVar) * he * tint * 0.5;
					data[i * 4 + 3] = 0;
				}
				else {
					const float noise = SimplexNoise::noise(x * 0.01, y * 0.01, z * 0.01) * 0.5 + SimplexNoise::noise(x * 0.03, y * 0.03, z * 0.03) * 0.3 + SimplexNoise::noise(x * 0.07, y * 0.1, z * 0.07) * 0.1;
					data[i * 4 + 0] = 0;
					data[i * 4 + 1] = 0;
					data[i * 4 + 2] = 0;
					float l = abs(y - 200) * 0.01;
					data[i * 4 + 3] = noise * 0.4 + 1 - (x + 1) / (1.5 + 2 * x * x);
				}
			}
		}
	}

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_3D, id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, w, h, d, 0, GL_RGBA, GL_FLOAT, data);

	return 0;
}

unsigned int VoxelTexture::generateTextureComputed() {
	if(id)glDeleteTextures(1, &id);
	w = 256;
	h = 128;
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
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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