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
#include "tileCreator/tileCreator.h"

void getBuildMode(GLFWwindow* window, tileCreator& tc);
bool processInput(GLFWwindow* window, tileCreator& tc, float blockSize);

//********************************************************************
//                         GLOBAL VARS
//********************************************************************
const int tileSize = 16;
bool buildMode = false;
bool verticesChanged = false;
const float blockSize = 0.1;
const int MAX_VERTS = 1000 * 5 * 6;
bool pKeyPressed = false;
bool lKeyPressed = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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

    unsigned int texture, playerSprite;
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

    glGenTextures(1, &playerSprite);
    glBindTexture(GL_TEXTURE_2D, playerSprite);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* data2 = stbi_load("player/playerSprite.png", &width, &height, &nrChannels, 4);

    if (data)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load player sprite" << std::endl;
    }
    myShader.setInt("playerSprite", 1);
    stbi_image_free(data2);

    //********************************************************************
    //                         Game Loop
    //********************************************************************

    tileCreator tc;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input processing here:
        getBuildMode(window, tc);
        if (buildMode)
        {
            verticesChanged = processInput(window, tc, blockSize);
        }

        if (verticesChanged)
        {
            tc.updateVertexBuffer();

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, tc.verticeVector.size() * sizeof(float), tc.verticeVector.data(), GL_STATIC_DRAW);
            verticesChanged = false;
        }
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        myShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!tc.verticeVector.empty()) {
            glDrawArrays(GL_TRIANGLES, 0, tc.verticeVector.size() / 5);

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

void getBuildMode(GLFWwindow* window, tileCreator& tc) 
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!pKeyPressed)
        {
            tc.writeToFile(tc.placedTiles, "data.txt");
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
            tc.readFromFile(tc.placedTiles, "data.txt");
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
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        buildMode = false;
    }

}

bool processInput(GLFWwindow* window, tileCreator& tc, float blockSize)
{
    static bool mouseHeld = false;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float ndcX = (xpos / width) * 2.0 - 1.0;
    float ndcY = 1.0 - (ypos / height) * 2.0; // Y is flipped due to opengl origin being bottom left.

    float snappedX = floor(ndcX / blockSize) * blockSize + blockSize / 2.0f;
    float snappedY = floor(ndcY / blockSize) * blockSize + blockSize / 2.0f;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) tc.selectTile(tileCreator::Grass);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) tc.selectTile(tileCreator::Side);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) tc.selectTile(tileCreator::Dirt);
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) tc.selectTile(tileCreator::Stone);
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) tc.selectTile(tileCreator::Coal);
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) tc.selectTile(tileCreator::Iron);
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) tc.selectTile(tileCreator::Gravel);
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) tc.selectTile(tileCreator::Sand);
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) tc.selectTile(tileCreator::Water);
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) tc.selectTile(tileCreator::Lava);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseHeld)
    {
        if (tc.placeTile(snappedX, snappedY, blockSize))
        {
            return true;
        }
    }
    else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !mouseHeld)
    {
        if (tc.removeTile(snappedX, snappedY))
        {
            return true;
        }
    }


    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        mouseHeld = false;
    }

    return false;
}