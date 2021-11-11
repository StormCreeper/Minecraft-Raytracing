#include "VoxelTexture.h"
#include "VoxelTexture.h"
#include "utils.h"
#include <iostream>
#include <GLFW/glfw3.h>
#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"

VoxelTexture::VoxelTexture() {
	texture_id = 0;
	shader_id = 0;
}

void VoxelTexture::init(int w, int h, int d) {

	dim[0] = w;
	dim[1] = h;
	dim[2] = d;
}

void VoxelTexture::generateTextureComputed() {

	if (shader_id) glDeleteProgram(shader_id);

	char* shaderSource = readSource("compute.glsl");
	unsigned int compute_shader = createShader(GL_COMPUTE_SHADER, shaderSource);

	free(shaderSource);

	if (compute_shader == 128) return;

	shader_id = glCreateProgram();

	glAttachShader(shader_id, compute_shader);
	glLinkProgram(shader_id);
	glDeleteShader(compute_shader);

	glUseProgram(shader_id);

	if (!texture_id) {
		glGenTextures(1, &texture_id);
	}

	glBindTexture(GL_TEXTURE_3D, texture_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dim[0], dim[1], dim[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);

	glBindImageTexture(0, texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glDispatchCompute(dim[0]/16, dim[1]/16, dim[2]/4);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI);
	glBindTexture(GL_TEXTURE_3D, 0);

	glUseProgram(0);

	uint8_t* data = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(unsigned int)));
	
	glBindTexture(GL_TEXTURE_3D, texture_id);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);

	if (runTimeData) free(runTimeData);
	runTimeData = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(uint8_t)));
	if (runTimeData)
		memcpy(runTimeData, data, dim[0] * dim[1] * dim[2] * sizeof(uint8_t));

	free(data);
}

int index(int x, int y, int z, int w, int h, int d) {
	return x + y * w + z * (w * h);
}

void VoxelTexture::generateMiniTexture() {
	srand(192901);

	uint8_t* data = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(unsigned int)));

	for (int x = 0; x < dim[0]; x++) {
		for (int y = 0; y < dim[1]; y++) {
			for (int z = 0; z < dim[2]; z++) {
				data[index(x, y, z, dim[0], dim[1], dim[2])] = rand() % 256;
			}
		}
	}
	if (!texture_id) {
		glGenTextures(1, &texture_id);
	}
	glBindTexture(GL_TEXTURE_3D, texture_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dim[0], dim[1], dim[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);

	if (runTimeData) free(runTimeData);
	runTimeData = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(uint8_t)));
	if(runTimeData)
		memcpy(runTimeData, data, dim[0] * dim[1] * dim[2] * sizeof(uint8_t));

	free(data);
}

void VoxelTexture::LoadFromVoxFile(const char* filename) {

	if (!texture_id) {
		glGenTextures(1, &texture_id);
	}

	FILE* fp;
	fopen_s(&fp, filename, "rb");

	if (!fp) {
		std::cerr << "Error opening the file " << filename << "\n";
		return;
	}

	fseek(fp, 0, SEEK_END);
	long buffer_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	uint8_t* buffer = new uint8_t[buffer_size];

	fread(buffer, buffer_size, 1, fp);
	fclose(fp);

	const ogt_vox_scene* scene = ogt_vox_read_scene(buffer, buffer_size);
	delete[] buffer;

	dim[0] = scene->models[0]->size_x;
	dim[1] = scene->models[0]->size_y;
	dim[2] = scene->models[0]->size_z;

	uint8_t* data = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(unsigned int)));

	if (!data) {
		std::cerr << "Unable to allocate enough memory\n";
		return;
	}
	for (int x = 0; x < dim[0]; x++) {
		for (int z = 0; z < dim[2]; z++) {
			for (int y = 0; y < dim[1]; y++) {
				unsigned int color_index = scene->models[0]->voxel_data[index(x, z, y, dim[0], dim[2], dim[1])];
				int i = index(x, y, z, dim[0], dim[1], dim[2]);
				
				data[i] = color_index;
			}
		}
	}

	for (int i = 0; i < 256; i++) {
		ogt_vox_rgba color = scene->palette.color[i];
		palette[i] = vec3((float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f);
	}

	ogt_vox_destroy_scene(scene);

	glBindTexture(GL_TEXTURE_3D, texture_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dim[0], dim[1], dim[2], 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_3D, 0);
	
	if (runTimeData) free(runTimeData);
	runTimeData = static_cast<uint8_t*>(malloc(dim[0] * dim[1] * dim[2] * sizeof(uint8_t)));
	if (runTimeData)
		memcpy(runTimeData, data, dim[0] * dim[1] * dim[2] * sizeof(uint8_t));

	free(data);
}

VoxelTexture::~VoxelTexture() {
	free(runTimeData);
	glDeleteTextures(1, &texture_id);
	glDeleteProgram(shader_id);
}
