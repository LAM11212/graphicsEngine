#include "mapManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>

mapManager::mapManager() {};

std::vector <tileCreator::Tile> mapManager::createMap(const std::string& name)
{
	maps.push_back({});
	mapNames.push_back(name);
	currentMapIndex = static_cast<int>(maps.size()) - 1; 
	return maps.back();
}

void mapManager::switchTo(int index)
{
	currentMapIndex = index;
}

std::vector<tileCreator::Tile>& mapManager::currentMap()
{
	return maps[currentMapIndex];
}