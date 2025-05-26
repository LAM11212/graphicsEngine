#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
#include "tileEditor/tileCreator.h"
#include "mapManager/mapManager.h"
#include <algorithm>
#include "GuiManager/guiManager.h"

void getBuildMode(GLFWwindow* window, tileCreator& tc);
bool processInput(GLFWwindow* window, tileCreator& tc, float blockSize, mapManager& mm);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//********************************************************************
//                         GLOBAL VARS
//********************************************************************
const int tileSize = 16;
bool buildMode = false;
bool verticesChanged = false;
int blockSize = 20;
const int MAX_VERTS = 1000 * 5 * 6;
bool pKeyPressed = false;
bool lKeyPressed = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool gridEnabled = false;
int mapCount = 0;
int windowWidth = 1200;
int windowHeight = 800;
float zoom = 1.0f;
bool notHeld = false;
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
    window = glfwCreateWindow(1200, 800, "IDKENGINE", NULL, NULL);
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

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader myShader("shaders/texture1.vs", "shaders/texture1.fs");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
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


    tileCreator tc(VBO);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    glm::vec3 translation(0, 0, 0);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));

    tileCreator::UV uv;
    mapManager mm;
    mm.currentMapIndex = 0;

    guiManager gui;

    ImTextureID my_tex_id = (ImTextureID)(intptr_t)texture;
    ImVec2 size = ImVec2(32.0f, 32.0f);
    ImVec4 bg_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    mm.createMap("Map_0", VBO);

//********************************************************************
//                         Game Loop
//********************************************************************

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        if (windowWidth == 0 || windowHeight == 0)
        {
            return -1;
        }

        glm::mat4 proj = glm::ortho(0.0f, float(windowWidth) / zoom, 0.0f, float(windowHeight) / zoom, -1.0f, 1.0f);

        tileCreator& activeMap = mm.currentMap();
        
        // input processing here:
        getBuildMode(window, mm.currentMap());
        if (buildMode)
        {
            verticesChanged = processInput(window, mm.currentMap(), blockSize, mm);
        }

        if (verticesChanged)
        {
            activeMap.updateVertexBuffer();

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, activeMap.verticeVector.size() * sizeof(float), activeMap.verticeVector.data(), GL_STATIC_DRAW);
            verticesChanged = false;
        }
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 mvp = proj * view * model;

        myShader.use();
        myShader.setMat4("projection", mvp);

        glBindVertexArray(VAO);

        myShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!activeMap.verticeVector.empty()) {
            glDrawArrays(GL_TRIANGLES, 0, activeMap.verticeVector.size() / 5);

        }

        gui.draw(activeMap, mm, zoom, blockSize, windowWidth, windowHeight, proj, my_tex_id, VBO);
        
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

//********************************************************************
//                         Process Inputs
//********************************************************************

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

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

bool processInput(GLFWwindow* window, tileCreator& tc, float blockSize, mapManager& mm)
{
    static bool mouseHeld;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float worldX = static_cast<float>(xpos) / zoom;
    float worldY = static_cast<float>(height - ypos) / zoom;

    float snappedX = floor(worldX / blockSize) * blockSize + blockSize / 2.0f;
    float snappedY = floor(worldY / blockSize) * blockSize + blockSize / 2.0f;

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

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            mouseHeld = true;
            if (mm.currentMap().placeTile(snappedX, snappedY, blockSize))
            {
                if (mouseHeld)
                {
                    mm.currentMap().chunk(snappedX, snappedY, blockSize);
                }
                return true;
            }
        }
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            if (mm.currentMap().removeTile(snappedX, snappedY))
            {
                return true;
            }
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        if (!tc.pushBackChunks.empty())
        {
            tc.chunks.push_back(tc.pushBackChunks);
            tc.pushBackChunks.clear();
        }
        mouseHeld = false;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS &&
        glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        if (!notHeld)
        {
            tc.undoLastChunk();
        }
        notHeld = true;
    }
    else
    {
        notHeld = false;
    }
    return false;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

    if (yoffset != 0)
    {
        float zoomFactor = 1.1f;
        if (yoffset > 0)
        {
            zoom *= zoomFactor;
        }
        else
        {
            zoom /= zoomFactor;
        }
    }
}