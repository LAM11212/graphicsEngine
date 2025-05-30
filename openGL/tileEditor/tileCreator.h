#pragma once
#ifndef TILE_CREATOR_H
#define TILE_CREATOR_H
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

class tileCreator
{
public:
	tileCreator(unsigned int tileVBO);

	struct Tile {
		glm::vec2 position;
		std::vector<float> vertices;
	};

	struct UV {
		float uMin, uMax, vMin, vMax;
	};

	int index = 0;

	enum tileType {
		Grass,
		Side,
		Dirt,
		Stone,
		Coal,
		Iron,
		Gravel,
		Sand,
		Water,
		Lava,
		Glass,
		Oak,
		Leaves,
		Torch,
		Rose
	};
	std::vector<Tile> placedTiles;
	std::vector<std::vector<Tile>> chunks;
	std::vector<Tile> pushBackChunks;
	std::vector<float> verticeVector;
	const int numOfTileTypes = 15;
	int prevChunkSize = 0;

	std::vector<float> makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax);
	void writeToFile(const std::vector<Tile>& allTiles, std::string fileName);
	void readFromFile(std::vector<Tile>& allTiles, const std::string fileName);
	UV calculateUV(int coordX, int coordY);
	bool placeTile(float x, float y, float blockSize);
	bool removeTile(float x, float y);
	void selectTile(tileType type);
	bool chunk(float x, float y, float blockSize);
	void updateVertexBuffer();
	void drawGrid(float width, float height, float blockSize, const glm::mat4& projection);
	UV getTileUVs(int num);
	void clear();
	const char* getStringFromEnum(tileCreator::tileType type);
	void undoLastChunk();

	tileType selectedTile = Grass;

private:
	unsigned int tileVBO;
	static constexpr int TILE_WIDTH = 16;
	static constexpr int TILE_HEIGHT = 16;
	static constexpr float UVSize = 1.0f / TILE_HEIGHT;

	// Texture atlas coordinates for each tile type
	static constexpr int sideX = 1, sideY = 15;
	static constexpr int grassX = 0, grassY = 15;
	static constexpr int dirtX = 2, dirtY = 15;
	static constexpr int stoneX = 3, stoneY = 15;
	static constexpr int coalX = 4, coalY = 15;
	static constexpr int ironX = 5, ironY = 15;
	static constexpr int gravelX = 6, gravelY = 15;
	static constexpr int sandX = 0, sandY = 14;
	static constexpr int waterX = 0, waterY = 0;
	static constexpr int lavaX = 0, lavaY = 1;
	static constexpr int glassX = 1, glassY = 14;
	static constexpr int oakX = 2, oakY = 14;
	static constexpr int leavesX = 4, leavesY = 14;
	static constexpr int torchX = 0, torchY = 13;
	static constexpr int roseX = 0, roseY = 12;
};

#endif