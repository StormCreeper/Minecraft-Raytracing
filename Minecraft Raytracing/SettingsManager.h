#pragma once

#include <variant>
#include "utils.h"

using vartype = std::variant<int, float, glm::ivec3, glm::vec2, glm::vec3>;

enum Type
{
	Int, Float, Ivec3, Vec2, Vec3, Color
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
};