#include "tileCreator.h"
#include "../Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>

tileCreator::tileCreator(unsigned int tileVBO) : tileVBO(tileVBO) {}

std::vector<float> tileCreator::makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax)
{
    return {
        //       x                   y         z      u     v
        -0.5f * blockSize, -0.5f * blockSize, 0.0f, uMin, vMin,
         0.5f * blockSize, -0.5f * blockSize, 0.0f, uMax, vMin,
         0.5f * blockSize,  0.5f * blockSize, 0.0f, uMax, vMax,

         0.5f * blockSize,  0.5f * blockSize, 0.0f, uMax, vMax,
        -0.5f * blockSize,  0.5f * blockSize, 0.0f, uMin, vMax,
        -0.5f * blockSize, -0.5f * blockSize, 0.0f, uMin, vMin
    };
}

void tileCreator::writeToFile(const std::vector<Tile>& allTiles, std::string fileName)
{
    std::ofstream file;
    file.open(fileName);

    if (file.is_open())
    {
        for (int i = 0; i < allTiles.size(); i++)
        {
            const Tile& newTile = allTiles[i];
            for (int j = 0; j < newTile.vertices.size(); j += 5)
            {
                file
                    << newTile.vertices[j + 0] << ", "
                    << newTile.vertices[j + 1] << ", "
                    << newTile.vertices[j + 2] << ", "
                    << newTile.vertices[j + 3] << ", "
                    << newTile.vertices[j + 4] << '\n';
            }
        }
        file.close();
        std::cout << "data written successfully" << std::endl;
    }
    else {
        std::cout << "failed to open file." << std::endl;
    }
}

void tileCreator::readFromFile(std::vector<Tile>& allTiles, const std::string fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cout << "failed to opne file." << std::endl;
        return;
    }

    std::string line;
    std::vector<float> tempVerts;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string val;
        float f;

        for (int i = 0; i < 5; i++)
        {
            std::getline(ss, val, ',');
            try
            {
                f = std::stof(val);
                tempVerts.push_back(f);
            }
            catch (std::exception e)
            {
                std::cout << e.what() << std::endl;
                break;
            }
        }

        if (tempVerts.size() == 30)
        {
            Tile newTile;
            newTile.vertices = tempVerts;

            newTile.position.x = newTile.vertices[0];
            newTile.position.y = newTile.vertices[1];

            allTiles.push_back(newTile);
            tempVerts.clear();
        }
    }
    file.close();
    std::cout << "loaded " << allTiles.size() << " tile correctly. yahooo!" << std::endl;
}

tileCreator::UV tileCreator::calculateUV(int coordX, int coordY)
{
    float uMin = coordX * UVSize;
    float uMax = uMin + UVSize;
    float vMin = coordY * UVSize;
    float vMax = vMin + UVSize;
    return { uMin, uMax, vMin, vMax };
}

void tileCreator::selectTile(tileType type)
{
    selectedTile = type;
}

bool tileCreator::placeTile(float x, float y, float blockSize)
{
    if (placedTiles.empty())
    {
        placedTiles.push_back({});
    }
    glm::vec2 pos = { x, y };

    for (const auto& tile : placedTiles) {
        if (tile.position == pos) return false;  // already placed
    }

    int texX = 0, texY = 0;
    switch (selectedTile) {
    case Grass: texX = grassX; texY = grassY; break;
    case Side: texX = sideX; texY = sideY; break;
    case Dirt: texX = dirtX; texY = dirtY; break;
    case Stone: texX = stoneX; texY = stoneY; break;
    case Coal: texX = coalX; texY = coalY; break;
    case Iron: texX = ironX; texY = ironY; break;
    case Gravel: texX = gravelX; texY = gravelY; break;
    case Sand: texX = sandX; texY = sandY; break;
    case Water: texX = waterX; texY = waterY; break;
    case Lava: texX = lavaX; texY = lavaY; break;
    case Glass: texX = glassX; texY = glassY; break;
    case Oak: texX = oakX; texY = oakY; break;
    case Leaves: texX = leavesX; texY = leavesY; break;
    case Torch: texX = torchX; texY = torchY; break;
    case Rose: texX = roseX; texY = roseY; break;
    }

    UV uv = calculateUV(texX, texY);
    std::vector<float> baseTile = makeBaseTile(blockSize, uv.uMin, uv.uMax, uv.vMin, uv.vMax);

    Tile newTile;
    newTile.position = pos;
    for (int i = 0; i < 6; ++i) {
        int idx = i * 5;
        newTile.vertices.push_back(baseTile[idx + 0] + x); // X
        newTile.vertices.push_back(baseTile[idx + 1] + y); // Y
        newTile.vertices.push_back(baseTile[idx + 2]);     // Z
        newTile.vertices.push_back(baseTile[idx + 3]);     // U
        newTile.vertices.push_back(baseTile[idx + 4]);     // V
    }

    placedTiles.push_back(newTile);
    return true;
}

bool tileCreator::removeTile(float x, float y)
{
    glm::vec2 checkPos = { x, y };

    for (auto x = placedTiles.begin(); x != placedTiles.end(); x++)
    {
        if (x->position == checkPos)
        {
            placedTiles.erase(x);
            break;
        }
    }
    return true;
}

void tileCreator::updateVertexBuffer()
{
    verticeVector.clear();
    for (const Tile& tile : placedTiles)
    {
        verticeVector.insert(verticeVector.end(), tile.vertices.begin(), tile.vertices.end());
    }

    glBindBuffer(GL_ARRAY_BUFFER, tileVBO);
    glBufferData(GL_ARRAY_BUFFER, verticeVector.size() * sizeof(float), verticeVector.data(), GL_STATIC_DRAW);
}

void tileCreator::drawGrid(float width, float height, float blockSize, const glm::mat4& projection)
{
    std::vector<float> lines;

    for (float x = 0; x <= width; x += blockSize)
    {
        lines.push_back(x);
        lines.push_back(0.0f);
        lines.push_back(0.0f);

        lines.push_back(x);
        lines.push_back(height);
        lines.push_back(0.0f);
    }

    for (float y = 0; y <= height; y += blockSize)
    {
        lines.push_back(0.0f);
        lines.push_back(y);
        lines.push_back(0.0f);

        lines.push_back(width);
        lines.push_back(y);
        lines.push_back(0.0f);
    }

    Shader gridShader("tileEditor/grid.vs", "tileEditor/grid.fs");

    unsigned int vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    gridShader.use();
    gridShader.setMat4("projection", projection);
    glDrawArrays(GL_LINES, 0, lines.size() / 3);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

}

tileCreator::UV tileCreator::getTileUVs(int num)
{
    
    int texX = 0, texY = 0;
    switch (num) {
    case 0: texX = grassX; texY = grassY; break;
    case 1: texX = sideX; texY = sideY; break;
    case 2: texX = dirtX; texY = dirtY; break;
    case 3: texX = stoneX; texY = stoneY; break;
    case 4: texX = coalX; texY = coalY; break;
    case 5: texX = ironX; texY = ironY; break;
    case 6: texX = gravelX; texY = gravelY; break;
    case 7: texX = sandX; texY = sandY; break;
    case 8: texX = waterX; texY = waterY; break;
    case 9: texX = lavaX; texY = lavaY; break;
    case 10: texX = glassX; texY = glassY; break;
    case 11: texX = oakX; texY = oakY; break;
    case 12: texX = leavesX; texY = leavesY; break;
    case 13: texX = torchX; texY = torchY; break;
    case 14: texX = roseX; texY = roseY; break;
    }

    UV uv = calculateUV(texX, texY);

    return uv;
}

void tileCreator::clear()
{
    placedTiles.clear();
    verticeVector.clear();
    updateVertexBuffer();
}

const char* tileCreator::getStringFromEnum(tileCreator::tileType type) 
{
    switch (type) {
    case tileCreator::Grass: return "Grass";
    case tileCreator::Side: return "Side";
    case tileCreator::Dirt: return "Dirt";
    case tileCreator::Stone: return "Stone";
    case tileCreator::Coal: return "Coal";
    case tileCreator::Iron: return "Iron";
    case tileCreator::Gravel: return "Gravel";
    case tileCreator::Sand: return "Sand";
    case tileCreator::Water: return "Water";
    case tileCreator::Lava: return "Lava";
    case tileCreator::Glass: return "Glass";
    case tileCreator::Oak: return "Oak";
    case tileCreator::Leaves: return "Leaves";
    case tileCreator::Torch: return "Torch";
    case tileCreator::Rose: return "Rose";
    default: return "Unknown";
    }
}