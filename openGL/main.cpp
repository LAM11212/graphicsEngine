#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

void getBuildMode(GLFWwindow* window);
bool processInput(GLFWwindow* window);

std::vector<float> makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax);

struct UV {
    float uMin, uMax, vMin, vMax;
};

UV calculateUV(int coordX, int coordY);

struct Tile {
    glm::vec2 position;
    std::vector<float> vertices;
};

std::vector<Tile> placedTiles;

void writeToFile(const std::vector<Tile>& allTiles, std::string fileName);
void readFromFile(std::vector<Tile>& allTiles, const std::string fileName);
//********************************************************************
//                         GLOBAL VARS
//********************************************************************
const int tileSize = 16;
bool buildMode = false;
bool verticesChanged = false;
const float blockSize = 0.1;
float currentX = -0.5f;
float currentY = -0.5f;
float UVSize = 1.0f / tileSize;
const int MAX_VERTS = 1000 * 5 * 6;
bool pKeyPressed = false;
bool lKeyPressed = false;
//********************************************************************
//                          Tile Enum
//********************************************************************
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
    Lava
};
tileType selectedTile = Side;
//********************************************************************
//                         Texture Atlas vars
//********************************************************************
// KEEP IN MIND THAT WE CALL stbi_set_flip_vertically_on_load(true); 
// THUS INVERTING THE BLOCKS.PNG ATLAS, MAKING THE GRASS BLOCK AT (0,0)
// BE CONVERTED TO (0, 15) I.E (just add 15 to the y value)
// WE DO THIS BECAUSE IF UNFLIPPED THE TEXTURE RENDERS UPSIDE DOWN.

const int sideX = 1;     // grass block side
const int sideY = 15;    // grass block side

const int grassX = 0;    // full grass block
const int grassY = 15;   // full grass block

const int dirtX = 2;     // dirt block
const int dirtY = 15;    // dirt block

const int stoneX = 3;    // stone block
const int stoneY = 15;   // stone block

const int coalX = 4;     // coal block
const int coalY = 15;    // coal block

const int ironX = 5;     // iron block
const int ironY = 15;    // iron block

const int gravelX = 6;   // gravel block
const int gravelY = 15;  // gravel block

const int sandX = 0;     // sand block
const int sandY = 14;    // sand block

const int waterX = 0;    // water block
const int waterY = 0;    // water block

const int lavaX = 0;     // lava block
const int lavaY = 1;     // lava block

UV calculateUV(int coordX, int coordY)
{
    float uMin = coordX * UVSize;
    float uMax = uMin + UVSize;
    float vMin = coordY * UVSize;
    float vMax = vMin + UVSize;
    return { uMin, uMax, vMin, vMax };
}

//********************************************************************
//                         initial vertice
//********************************************************************

std::vector<float> makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax)
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
std::vector<float> verticeVector;

//********************************************************************
//                         Main Function
//********************************************************************

int main(void)
{

    //********************************************************************
    //                      Window Initialization
    //********************************************************************

    GLFWwindow* window;
    if (!glfwInit())
        return -1;


    // Create window and OpenGL context
    window = glfwCreateWindow(1200, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Set context before loading GLAD
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    Shader myShader("shaders/texture1.vs", "shaders/texture1.fs");

    
    //********************************************************************
    //                         VAO/VBO Initialization
    //********************************************************************

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTS * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    //position pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //********************************************************************
    //                         Texture Initialization
    //********************************************************************

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("textures/blocks.png", &width, &height, &nrChannels, 4);

    if (data) 
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else 
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    myShader.setInt("texture1", 0);
    
    stbi_image_free(data);

    //********************************************************************
    //                         Game Loop
    //********************************************************************

    while (!glfwWindowShouldClose(window))
    {
        // input processing here:
        getBuildMode(window);
        
        if (buildMode)
        {
            verticesChanged = processInput(window);
        }

        if (verticesChanged)
        {
            verticeVector.clear();

            for (const Tile& tiles : placedTiles)
            {
                verticeVector.insert(verticeVector.end(), tiles.vertices.begin(), tiles.vertices.end());
            }
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verticeVector.size() * sizeof(float), verticeVector.data(), GL_STATIC_DRAW);
            std::cout << "Buffer updated\n";
            verticesChanged = false;
            
        }
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        myShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!verticeVector.empty()) {
            glDrawArrays(GL_TRIANGLES, 0, verticeVector.size() / 5);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

//********************************************************************
//                         Process Inputs
//********************************************************************

void getBuildMode(GLFWwindow* window) 
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!pKeyPressed)
        {
            writeToFile(placedTiles, "data.txt");
        }
        pKeyPressed = true;
    }
    else
    {
        pKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        if (!lKeyPressed)
        {
            readFromFile(placedTiles, "data.txt");
        }
        lKeyPressed = true;
    }
    else
    {
        lKeyPressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        buildMode = true;
        std::cout << "build mode enabled" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        buildMode = false;
        std::cout << "build mode disabled" << std::endl;
    }

}

bool processInput(GLFWwindow* window)
{
    static bool mouseHeld = false;
    double xpos, ypos;
    std::vector<float> baseTile;

    if (glfwGetKey(window, GLFW_KEY_1))
    {
        selectedTile = Grass;
    }
    else if (glfwGetKey(window, GLFW_KEY_2))
    {
        selectedTile = Side;
    }
    else if (glfwGetKey(window, GLFW_KEY_3))
    {
        selectedTile = Dirt;
    }
    else if (glfwGetKey(window, GLFW_KEY_4))
    {
        selectedTile = Stone;
    }
    else if (glfwGetKey(window, GLFW_KEY_5))
    {
        selectedTile = Coal;
    }
    else if (glfwGetKey(window, GLFW_KEY_6))
    {
        selectedTile = Iron;
    }
    else if (glfwGetKey(window, GLFW_KEY_7))
    {
        selectedTile = Gravel;
    }
    else if (glfwGetKey(window, GLFW_KEY_8))
    {
        selectedTile = Sand;
    }
    else if (glfwGetKey(window, GLFW_KEY_9))
    {
        selectedTile = Water;
    }
    else if (glfwGetKey(window, GLFW_KEY_0))
    {
        selectedTile = Lava;
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseHeld)
    {

        UV uv;
        if (selectedTile == Grass)
        {
            uv = calculateUV(grassX, grassY);
        }
        else if (selectedTile == Side)
        {
            uv = calculateUV(sideX, sideY);
        }
        else if (selectedTile == Dirt)
        {
            uv = calculateUV(dirtX, dirtY);
        }
        else if (selectedTile == Stone)
        {
            uv = calculateUV(stoneX, stoneY);
        }
        else if (selectedTile == Coal)
        {
            uv = calculateUV(coalX, coalY);
        }
        else if (selectedTile == Iron)
        {
            uv = calculateUV(ironX, ironY);
        }
        else if (selectedTile == Gravel)
        {
            uv = calculateUV(gravelX, gravelY);
        }
        else if (selectedTile == Sand)
        {
            uv = calculateUV(sandX, sandY);
        }
        else if (selectedTile == Water)
        {
            uv = calculateUV(waterX, waterY);
        }
        else if (selectedTile == Lava)
        {
            uv = calculateUV(lavaX, lavaY);
        }

        baseTile = makeBaseTile(blockSize, uv.uMin, uv.uMax, uv.vMin, uv.vMax);
        
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float ndcX = (xpos / width) * 2.0 - 1.0;
        float ndcY = 1.0 - (ypos / height) * 2.0; // Y is flipped due to opengl origin being bottom left.

        float snappedX = floor(ndcX / blockSize) * blockSize + blockSize / 2.0f;
        float snappedY = floor(ndcY / blockSize) * blockSize + blockSize / 2.0f;


        Tile newTile;
        newTile.position = { snappedX, snappedY };

        bool tileExists = false;
        for (const Tile& tile : placedTiles)
        {
            if (tile.position == newTile.position)
            {
                tileExists = true;
                break;
            }
        }

        if (!tileExists)
        {
            for (int i = 0; i < 6; i++)
            {
                int idx = i * 5;
                newTile.vertices.push_back(baseTile[idx + 0] + snappedX);    // x
                newTile.vertices.push_back(baseTile[idx + 1] + snappedY);    // y
                newTile.vertices.push_back(baseTile[idx + 2]);               // z
                newTile.vertices.push_back(baseTile[idx + 3]);               // u
                newTile.vertices.push_back(baseTile[idx + 4]);               // v
            }
            placedTiles.push_back(newTile);
            mouseHeld = true;
            return true;
        }
    }
    else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        mouseHeld = false;
    }


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !mouseHeld)
    {

        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float ndcX = (xpos / width) * 2.0 - 1.0;
        float ndcY = 1.0 - (ypos / height) * 2.0; // Y is flipped due to opengl origin being bottom left.

        float snappedX = floor(ndcX / blockSize) * blockSize + blockSize / 2.0f;
        float snappedY = floor(ndcY / blockSize) * blockSize + blockSize / 2.0f;

        glm::vec2 checkPos = { snappedX, snappedY };

        for (auto x = placedTiles.begin(); x != placedTiles.end(); x++)
        {
            if (x->position == checkPos)
            {
                placedTiles.erase(x);
                mouseHeld = true;
                break;
            }
        }
        return true;
    } 
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
    {
        mouseHeld = false;
    }
    return false;
}

//********************************************************************
//                         File I/O
//********************************************************************

void writeToFile(const std::vector<Tile>& allTiles, std::string fileName)
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

void readFromFile(std::vector<Tile>& allTiles, const std::string fileName)
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