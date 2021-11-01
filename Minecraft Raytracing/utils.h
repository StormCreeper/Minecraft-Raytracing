#pragma once

#include <GL/glew.h>
#include <vector>
#include <string>
#include "vf3dMat44.h"

unsigned int createShader(unsigned int shader_type, const char* shader_source, char** info = nullptr);
unsigned int createProgram(unsigned int vertex_shader, unsigned int fragment_shader);
unsigned int createProgram(const char* vert_name, const char* frag_name, char** info = nullptr);
void createProgram(unsigned int* shader, unsigned int vertex_shader, unsigned int fragment_shader);

char* readSource(const char* filename);

void setUniformM4(unsigned int shader, const char* name, mat44 matrix);
void setUniformM4(unsigned int shader, int loc, mat44 matrix);

void setUniformVec3(unsigned int shader, const char* name, float x, float y, float z);
void setUniformVec3(unsigned int shader, int location, float x, float y, float z);

void setUniformVec3(unsigned int shader, const char* name, v3 vector);

void setUniformVec2(unsigned int shader, const char* name, float x, float y);
void setUniformVec2(unsigned int shader, int location, float x, float y);

void setUniformInt(unsigned int shader, const char* name, int value);
void setUniformInt(unsigned int shader, int location, int value);

void setUniformFloat(unsigned int shader, const char* name, float value);
void setUniformFloat(unsigned int shader, int location, float value);