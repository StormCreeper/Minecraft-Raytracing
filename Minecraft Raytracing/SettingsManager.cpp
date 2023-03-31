#include "SettingsManager.h"
#include <utility>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <fstream>

std::vector<Setting> SettingsManager::settings;

void SettingsManager::load(std::string filename) {
	settings.clear();
	std::ifstream file;
	file.open(filename);
	if (file.is_open()) {
		
		while (!file.eof()) {
			std::string first;
			file >> first;
			if (first == "") continue;
			
			if (first == "#") {
				while (!file.eof() && first != ";") {
					file >> first;
				}
				continue;
			}
			if (first == "label") {
				std::string label = "";
				while (!file.eof() && first != ";") {
					file >> first;
					if(first != ";")
						label += first;
				}
				Setting setting{ label, Label, {} };
				settings.push_back(setting);
				continue;
			}

			std::string name;
			file >> name;

			Setting setting;
			setting.name = name;

			if (first == "int") {
				int i;
				file >> i;
				setting.type = Int;
				setting.value = vartype(i);
			}
			if (first == "float") {
				float f;
				file >> f;
				setting.type = Float;
				setting.value = vartype(f);
			}
			if (first == "ivec3") {
				int x, y, z;
				file >> x >> y >> z;
				setting.type = Ivec3;
				setting.value = vartype(glm::ivec3(x, y, z));
			}
			if (first == "vec2") {
				float x, y;
				file >> x >> y;
				setting.type = Vec2;
				setting.value = vartype(glm::vec2(x, y));
			}
			if (first == "vec3") {
				float x, y, z;
				file >> x >> y >> z;
				setting.type = Vec3;
				setting.value = vartype(glm::vec3(x, y, z));
			}
			if (first == "color") {
				float x, y, z;
				file >> x >> y >> z;
				setting.type = Color;
				setting.value = vartype(glm::vec3(x, y, z));
			}

			file >> setting.min >> setting.max;
			file >> first;

			settings.push_back(setting);
		}

		file.close();
	} else {
		std::cout << "Unable to open file " << filename << std::endl;
	}
}

void SettingsManager::save(std::string filename) {
	std::ofstream file;
	file.open(filename);
	if (file.is_open()) {
		for (Setting& setting : settings) {
			bool minmax = true;
			switch (setting.type) {
			case Int:
				file << "int " << setting.name << " " << std::get<int>(setting.value);
				break;
			case Float:
				file << "float " << setting.name << " " << std::get<float>(setting.value);
				break;
			case Ivec3: {
				glm::ivec3 vec = std::get<glm::ivec3>(setting.value);
				file << "ivec3 " << setting.name << " " << vec.x << " " << vec.y << " " << vec.z;
			}	break;
			case Vec2: {
				glm::vec2 vec = std::get<glm::vec2>(setting.value);
				file << "vec2 " << setting.name << " " << vec.x << " " << vec.y;
			}	break;
			case Vec3: {
				glm::vec3 vec = std::get<glm::vec3>(setting.value);
				file << "vec3 " << setting.name << " " << vec.x << " " << vec.y << " " << vec.z;
			}	break;
			case Color: {
				glm::vec3 vec = std::get<glm::vec3>(setting.value);
				file << "color " << setting.name << " " << vec.x << " " << vec.y << " " << vec.z;
			}	break;
			case Label:
				file << "label " << setting.name << " ;\n";
				minmax = false;
				break;
			default:
				break;
			}
			if(minmax)
				file << " " << setting.min << " " << setting.max << " ;" << std::endl;
		}
		file.close();
	}
	else {
		std::cout << "Unable to open file " << filename << std::endl;
	}
}

void SettingsManager::ImGuiWindow() {
	for (Setting& setting : settings) {
		switch (setting.type) {
		case Int:
			{int i = std::get<int>(setting.value);
			ImGui::SliderInt(setting.name.c_str(), &i, setting.min, setting.max);
			setting.value.emplace<int>(i); }
			break;
		case Float:
			{float f = std::get<float>(setting.value);
			ImGui::SliderFloat(setting.name.c_str(), &f, setting.min, setting.max);
			setting.value.emplace<float>(f); }
			break;
		case Ivec3:
			{glm::ivec3 vec = std::get<glm::ivec3>(setting.value);
			int bufi[3] = { vec.x, vec.y, vec.z };
			ImGui::SliderInt3(setting.name.c_str(), bufi, setting.min, setting.max);
			setting.value.emplace<glm::ivec3>(glm::ivec3(bufi[0], bufi[1], bufi[2]));
			break; }
		case Vec2:
			{glm::vec2 vec = std::get<glm::vec2>(setting.value);
			float buff2[2] = { vec.x, vec.y };
			ImGui::SliderFloat2(setting.name.c_str(), buff2, setting.min, setting.max);
			setting.value.emplace<glm::vec2>(glm::vec2(buff2[0], buff2[1]));
			break; }
		case Vec3:
		case Color:
			{glm::vec3 vec = std::get<glm::vec3>(setting.value);
			float bufc[3] = { vec.x, vec.y, vec.z };
			ImGui::SliderFloat3(setting.name.c_str(), bufc, setting.min, setting.max);
			setting.value.emplace<glm::vec3>(glm::vec3(bufc[0], bufc[1], bufc[2]));
			break; }
		case Label:
			ImGui::Text(setting.name.c_str());
			break;
		default:
			break;
		}
	}
}

