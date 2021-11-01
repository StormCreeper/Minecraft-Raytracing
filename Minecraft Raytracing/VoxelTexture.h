#pragma once

#include "vf3dMat44.h"

class VoxelTexture {
public:
	int dim[3];

	unsigned char *data;
	float voxel_width;

	v3 palette[256];

public:
	VoxelTexture();

public:

	unsigned int texture_id;
	unsigned int shader_id;

	void init(int w, int h, int d);

	void generateTextureComputed();
	void generateMiniTexture();
	void LoadFromVoxFile(const char* filename);

	~VoxelTexture();
};
