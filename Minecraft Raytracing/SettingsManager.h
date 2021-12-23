#pragma once

#include <variant>
#include "utils.h"
#include <iostream>

using vartype = std::variant<int, float, glm::ivec3, glm::vec2, glm::vec3>;

enum Type
{
	Int, Float, Ivec3, Vec2, Vec3, Color, Label
};

struct Setting {
	std::string name;
	Type type;
	vartype value;
	float min, max;
};

class SettingsManager {
public:
	static std::vector<Setting> settings;

	static void load(std::string filename);
	static void save(std::string filename);

	static void ImGuiWindow();

	template<class T> static T get(std::string name);
	template<class T> static void set(std::string name, T value);
};

template<class T> inline T SettingsManager::get(std::string name) {
	for (Setting &setting : settings) {
		if (setting.name == name) {
			return std::get<T>(setting.value);
		}
	}
	std::cout << "Error, couldn't find " << name << std::endl;
	return T();
}

template<class T>
inline void SettingsManager::set(std::string name, T value) {
	for (Setting &setting : settings) {
		if (setting.name == name) {
			setting.value.emplace<T>(value);
			return;
		}
	}
	std::cout << "Error, couldn't find " << name << std::endl;
}
