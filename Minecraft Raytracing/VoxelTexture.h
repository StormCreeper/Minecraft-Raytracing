#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

int index(int x, int y, int z, int w, int h, int d);

class VoxelTexture {
public:
	int dim[3];

	uint8_t* runTimeData;
	float voxel_width;

	vec3 palette[256];

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
