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