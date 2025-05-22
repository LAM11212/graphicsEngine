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
	std::vector<tileCreator> maps;
	int currentMapIndex = 0;

	void createMap(const std::string& name);
	void switchTo(int index);
	tileCreator& currentMap();
};


#endif