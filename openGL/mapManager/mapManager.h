#pragma once
#ifndef MAP_MANAGER_H
#define MAP_MANAGER_H
#include <iostream>
#include <vector>
#include <string>
#include "../tileEditor/tileCreator.h"

class mapManager
{
public:
	mapManager();

	std::vector<std::string> mapNames;
	std::vector<std::vector<tileCreator::Tile>> maps;
	int currentMapIndex = 0;

	std::vector<tileCreator::Tile> createMap(const std::string& name);
	void switchTo(int index);
	std::vector<tileCreator::Tile>& currentMap();
};


#endif