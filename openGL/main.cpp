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
#include "tileCreator/tileCreator.h"

void getBuildMode(GLFWwindow* window, tileCreator& tc);
bool processInput(GLFWwindow* window, tileCreator& tc, float blockSize);

//********************************************************************
//                         GLOBAL VARS
//********************************************************************
const int tileSize = 16;
bool buildMode = false;
bool verticesChanged = false;
const float blockSize = 10.0f;
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

    glm::mat4 proj = glm::ortho(0.0f, 1200.0f, 0.0f, 800.0f, -1.0f, 1.0f);

    Shader myShader("shaders/texture1.vs", "shaders/texture1.fs");
    myShader.use();
    myShader.setMat4("projection", proj);
    
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

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");
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

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glBindVertexArray(VAO);

        myShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        if (!tc.verticeVector.empty()) {
            glDrawArrays(GL_TRIANGLES, 0, tc.verticeVector.size() / 5);

        }

        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        
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
    float worldX = static_cast<float>(xpos);
    float worldY = static_cast<float>(height - ypos);

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