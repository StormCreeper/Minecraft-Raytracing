#pragma once

class VoxelTexture {
public:
	int dim[3];
	int ldim[3];

	float* data;
	float voxel_width;

public:
	VoxelTexture();

public:

	unsigned int texture_id;
	unsigned int shader_id;

	void init();

	void generateTextureComputed();


	~VoxelTexture();
};
