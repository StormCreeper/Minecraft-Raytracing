#include "utils.h"
#include <iostream>

unsigned int createShader(unsigned int shader_type, const char* shader_source, char** info) {
	const unsigned int shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_source, nullptr);
	glCompileShader(shader);

	int success;

	char* infoLog;
	if (info) infoLog = *info;
	else infoLog = static_cast<char*>(malloc(2048 * (sizeof(char))));
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(shader, 2048, nullptr, infoLog);
		std::cerr << "Shader compilation error :\n" << infoLog << std::endl;
		return 128;
	}

	return shader;
}

unsigned int createProgram(const char* vert_name, const char* frag_name, char** info) {
	char* vertex_shader_source = readSource(vert_name);
	char* fragment_shader_source = readSource(frag_name);

	const unsigned int shader = createProgram(createShader(GL_VERTEX_SHADER, vertex_shader_source, info), createShader(GL_FRAGMENT_SHADER, fragment_shader_source, info));
	free(vertex_shader_source);
	free(fragment_shader_source);

	return shader;
}

void createProgram(unsigned int* shader, unsigned int vertex_shader, unsigned int fragment_shader) {
	*shader = createProgram(vertex_shader, fragment_shader);
}

unsigned int createProgram(unsigned int vertex_shader, unsigned int fragment_shader) {
	if (vertex_shader == 128 || fragment_shader == 128) {
		return 128;
	}
	const unsigned int shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);

	glLinkProgram(shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader_program;
}

char* readSource(const char* filename) {
	char* text = nullptr;

	if (filename != nullptr) {
		FILE* file;
		fopen_s(&file, filename, "rt");

		if (file != nullptr) {
			fseek(file, 0, SEEK_END);
			size_t count = ftell(file);
			rewind(file);

			if (count > 0) {
				text = static_cast<char*>(malloc(sizeof(char) * (count + 1)));
				count = fread(text, sizeof(char), count, file);
				text[count] = '\0';
			}
			fclose(file);
		}
	}
	else {
		std::cerr << "Nom de fichier invalide";
	}
	return text;
}
void setUniformM4(const unsigned int shader, const char* name, mat4 matrix) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(matrix));
}
void setUniformM4(unsigned int shader, int loc, mat4 matrix) {
	glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(matrix));
}
void setUniformVec3(const unsigned int shader, const char* name, const float x, const float y, const float z) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform3f(location, x, y, z);
}
void setUniformVec3(unsigned int shader, const int location, const float x, const float y, const float z) {
	glUniform3f(location, x, y, z);
}
void setUniformVec3i(const unsigned int shader, const char* name, const int x, const int y, const int z) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform3i(location, x, y, z);
}
void setUniformVec3i(unsigned int shader, const int location, const int x, const int y, const int z) {
	glUniform3i(location, x, y, z);
}
void setUniformVec3(const unsigned int shader, const char* name, const vec3 vector) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform3f(location, vector.x, vector.y, vector.z);
}
void setUniformVec2(const unsigned int shader, const char* name, const float x, const float y) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform2f(location, x, y);
}
void setUniformVec2(const unsigned int shader, const int location, const float x, const float y) {
	glUniform2f(location, x, y);
}
void setUniformInt(const unsigned int shader, const char* name, const int value) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform1i(location, value);
}
void setUniformInt(const unsigned int shader, const int location, const int value) {
	glUniform1i(location, value);
}
void setUniformFloat(const unsigned int shader, const char* name, const float value) {
	const unsigned int location = glGetUniformLocation(shader, name);
	glUniform1f(location, value);
}
void setUniformFloat(const unsigned int shader, const int location, const float value) {
	glUniform1f(location, value);
}


uint8_t queryVoxel(VoxelTexture& tex, int x, int y, int z) {
	if (x < 0 || y < 0 || z < 0) return 0;
	if (x >= tex.dim[0] || y >= tex.dim[1] || z >= tex.dim[2]) return 0;

	return tex.runTimeData[index(x, y, z, tex.dim[0], tex.dim[1], tex.dim[2])];
}

void setVoxel(VoxelTexture& tex, int x, int y, int z, uint8_t vox) {
	if (x < 0 || y < 0 || z < 0) return;
	if (x >= tex.dim[0] || y >= tex.dim[1] || z >= tex.dim[2]) return;

	GLuint data[1] = { vox };
	glBindTexture(GL_TEXTURE_3D, tex.texture_id);

	glTexSubImage3D(GL_TEXTURE_3D, 0, x, y, z, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
	tex.runTimeData[index(x, y, z, tex.dim[0], tex.dim[1], tex.dim[2])] = vox;
}

float voxel_traversal(VoxelTexture& tex, vec3 origin, vec3 direction, vec3& normal, uint8_t& block, int& mapX, int& mapY, int& mapZ) {

	mapX = int(floor(origin.x));
	mapY = int(floor(origin.y));
	mapZ = int(floor(origin.z));

	block = queryVoxel(tex, mapX, mapY, mapZ);
	if (block != 0 && block != 14) {
		normal = vec3(0, 0, 0);
		return 0;
	}

	float sideDistX;
	float sideDistY;
	float sideDistZ;

	float deltaDX = abs(1 / direction.x);
	float deltaDY = abs(1 / direction.y);
	float deltaDZ = abs(1 / direction.z);
	float perpWallDist = -1;

	int stepX;
	int stepY;
	int stepZ;

	int hit = 0;
	int side;

	if (direction.x < 0) {
		stepX = -1;
		sideDistX = (origin.x - mapX) * deltaDX;
	}
	else {
		stepX = 1;
		sideDistX = (float(mapX) + 1.0 - origin.x) * deltaDX;
	}
	if (direction.y < 0) {
		stepY = -1;
		sideDistY = (origin.y - mapY) * deltaDY;
	}
	else {
		stepY = 1;
		sideDistY = (float(mapY) + 1.0 - origin.y) * deltaDY;
	}
	if (direction.z < 0) {
		stepZ = -1;
		sideDistZ = (origin.z - mapZ) * deltaDZ;
	}
	else {
		stepZ = 1;
		sideDistZ = (float(mapZ) + 1.0 - origin.z) * deltaDZ;
	}

	int step = 1;

	for (int i = 0; i < 15; i++) {
		if ((mapX >= tex.dim[0] && stepX > 0) || (mapY >= tex.dim[1] && stepY > 0) || (mapZ >= tex.dim[2] && stepZ > 0)) break;
		if ((mapX < 0 && stepX < 0) || (mapY < 0 && stepY < 0) || (mapZ < 0 && stepZ < 0)) break;

		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDX;
			mapX += stepX * step;
			side = 0;
		}
		else if (sideDistY < sideDistX && sideDistY < sideDistZ) {
			sideDistY += deltaDY;
			mapY += stepY * step;
			side = 1;
		}
		else {
			sideDistZ += deltaDZ;
			mapZ += stepZ * step;
			side = 2;
		}

		block = queryVoxel(tex, mapX, mapY ,mapZ);
		if (block != 0 && block != 14) {
			if (side == 0) {
				perpWallDist = (mapX - origin.x + (1 - stepX * step) / 2) / direction.x;
				normal = vec3(1, 0, 0) * -float(stepX);
			}
			else if (side == 1) {
				perpWallDist = (mapY - origin.y + (1 - stepY * step) / 2) / direction.y;
				normal = vec3(0, 1, 0) * -float(stepY);
			}
			else {
				perpWallDist = (mapZ - origin.z + (1 - stepZ * step) / 2) / direction.z;
				normal = vec3(0, 0, 1) * -float(stepZ);
			}

			break;
		}
	}

	return perpWallDist;
}