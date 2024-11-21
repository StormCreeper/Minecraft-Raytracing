#include "SaveManager.h"
#include <iostream>
#include <fstream>

std::vector<Action> SaveManager::actions;

void SaveManager::save(std::string filename, Player& player) {
	std::ofstream file;
	file.open(filename);

	if (file.is_open()) {
		file << actions.size() << std::endl;

		for (Action a : actions) {
			file << a.block << " " << a.position.x << " " << a.position.y << " " << a.position.z << std::endl;
		}

		file << player.position.x << " " << player.position.y << " " << player.position.z << std::endl;

		file.close();
	} else {
		std::cout << "Unable to open file " << filename << std::endl;
	}
	
}

void SaveManager::load(std::string filename, Player& player, VoxelTexture* tex) {
	actions.clear();
	
	std::ifstream file;
	file.open(filename);

	if (file.is_open()) {
		int num;
		file >> num;

		for (int i = 0; i < num; i++) {
			Action a;
			file >> a.block >> a.position.x >> a.position.y >> a.position.z;
			actions.push_back(a);

			setVoxel(*tex, a.position.x, a.position.y, a.position.z, a.block);
		}

		file.close();
	} else {
		std::cout << "Unable to open file " << filename << std::endl;
	}
}

void SaveManager::addAction(glm::ivec3 position, int block) {
	bool found = false;
	for (Action& a : actions) {
		if (position == a.position) {
			a.block = block;
			found = true;
		}
	}
	if(!found)
		actions.push_back({ position, block });
}
