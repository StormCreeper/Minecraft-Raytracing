#pragma once
#include "utils.h"
#include "Player.h"


struct Action {
	glm::ivec3 position;
	int block;
};

class SaveManager {
public:
	static std::vector<Action> actions;
	static void save(std::string filename, Player &player);
	static void load(std::string filename, Player &player, VoxelTexture *tex);
	static void addAction(glm::ivec3 position, int block);
};