#pragma once

#include "SimplexNoise.h"

class VoxelTexture {
public:
	int w, h, d;

	float* data;
	float voxel_width;

public:
	VoxelTexture() {
		w = 64;
		h = 64;
		d = 64;
	}
	VoxelTexture(int ww, int yy, int zz) {
		w = ww;
		h = yy;
		d = zz;
	}

public:

	unsigned int id;
	
	unsigned int generateTerrainTexture();
	unsigned int generateTextureComputed();


	~VoxelTexture();
};
