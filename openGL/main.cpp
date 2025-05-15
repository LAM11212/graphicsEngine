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

void processInput(GLFWwindow* window);
bool processInput(GLFWwindow* window, std::vector<float>& vertices);

//********************************************************************
//                         GLOBAL VARS
//********************************************************************
bool buildMode = false;
bool verticesChanged = false;

//********************************************************************
//                         Texture Atlas vars
//********************************************************************
// KEEP IN MIND THAT WE CALL stbi_set_flip_vertically_on_load(true); 
// THUS INVERTING THE BLOCKS.PNG ATLAS, MAKING THE GRASS BLOCK AT (0,0)
// BE CONVERTED TO (0, 15) I.E (just add 15 to the y value)
// WE DO THIS BECAUSE IF UNFLIPPED THE TEXTURE RENDERS UPSIDE DOWN.


const int sideX = 1; // grass block side
const int sideY = 15; // grass block side
const int tileSize = 16;

float UVSize = 1.0f / tileSize;
float uMin = sideX * UVSize;
float uMax = uMin + UVSize;
float vMin = sideY * UVSize;
float vMax = vMin + UVSize;

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
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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

    //********************************************************************
    //                         Shader object/Vertices 
    //********************************************************************

    Shader myShader("shaders/texture1.vs", "shaders/texture1.fs");

    float vertices[] = 
    {
    //    x      y     z     u     v
        -0.5f, -0.5f, 0.0f, uMin, vMin,    // bottom left
         0.5f, -0.5f, 0.0f, uMax, vMin,    // bottom right 
         0.5f,  0.5f, 0.0f, uMax, vMax,    // top right
         0.5f,  0.5f, 0.0f, uMax, vMax,    // top right
        -0.5f,  0.5f, 0.0f, uMin, vMax,    // top left
        -0.5f, -0.5f, 0.0f, uMin, vMin     // bottom left     
    };
    int stride = 5;
    std::vector<float> verticeVector(std::begin(vertices), std::end(vertices));
    
    //********************************************************************
    //                         VAO/VBO Initialization
    //********************************************************************

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticeVector.size() * sizeof(float), verticeVector.data(), GL_DYNAMIC_DRAW);
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
        processInput(window);

        if (buildMode)
        {
            verticesChanged = processInput(window, verticeVector);
        }

        if (verticesChanged)
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verticeVector.size() * sizeof(float), verticeVector.data(), GL_DYNAMIC_DRAW);
            verticesChanged = false;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        myShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glDrawArrays(GL_TRIANGLES, 0, verticeVector.size() / 5);

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
void processInput(GLFWwindow* window) 
{

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

bool processInput(GLFWwindow* window, std::vector<float>& vertices)
{
    if (buildMode && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        std::cout << "right key triggered" << std::endl;
        int stride = 5;
        for (int i = 0; i < vertices.size(); i += stride)
        {
            vertices[i] += 0.5f;
        }
        return true;
    }
    else if (buildMode && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        std::cout << "left key triggered" << std::endl;
        int stride = 5;
        for (int i = 0; i < vertices.size(); i += stride)
        {
            vertices[i] -= 0.5f;
        }
        return true;
    }
    else if (buildMode && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        std::cout << "up key triggered" << std::endl;
        int stride = 5;
        for (int i = 0; i < vertices.size(); i += stride)
        {
            vertices[i + 1] += 0.5f;
        }
        return true;
    }
    else if (buildMode && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        std::cout << "down key triggered" << std::endl;
        int stride = 5;
        for (int i = 0; i < vertices.size(); i += stride)
        {
            vertices[i + 1] -= 0.5f;
        }
        return true;
    }
    else
    {
        return false;
    }
}