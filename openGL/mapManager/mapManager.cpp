#include "mapManager.h"
#include "../tileEditor/tileCreator.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>

mapManager::mapManager() {};

void mapManager::createMap(const std::string& name, unsigned int tileVBO)
{
	tileCreator newMap(tileVBO);
	newMap.updateVertexBuffer();
	maps.push_back(newMap);
	mapNames.push_back(name);
	currentMapIndex = static_cast<int>(maps.size()) - 1; 
}

void mapManager::switchTo(int index)
{
	currentMapIndex = index;
	maps[index].updateVertexBuffer();
}

tileCreator& mapManager::currentMap()
{
	return maps[currentMapIndex];
}