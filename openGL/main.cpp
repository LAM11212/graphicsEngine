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

const int MAX_VERTS = 1000 * 5 * 6;

void processInput(GLFWwindow* window);
bool processInput(GLFWwindow* window, std::vector<float>& vertices);
std::vector<float> makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax);
struct UV {
    float uMin, uMax, vMin, vMax;
};

UV calculateUV(int coordX, int coordY);


//********************************************************************
//                         GLOBAL VARS
//********************************************************************
const int tileSize = 16;
bool buildMode = false;
bool verticesChanged = false;
const float blockSize = 0.2;
float currentX = -0.5f;
float currentY = -0.5f;
float UVSize = 1.0f / tileSize;

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

const int dirtX = 3;     // dirt block
const int dirtY = 15;    // dirt block

UV calculateUV(int coordX, int coordY)
{
    float uMin = coordX * UVSize;
    float uMax = uMin + UVSize;
    float vMin = coordY * UVSize;
    float vMax = vMin + UVSize;
    return { uMin, uMax, vMin, vMax };
}



//********************************************************************
//                         initial vertices
//********************************************************************

std::vector<float> makeBaseTile(float blockSize, float uMin, float uMax, float vMin, float vMax)
{
    return {
        // x, y, z, u, v
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
        processInput(window);
        
        if (buildMode)
        {
            verticesChanged = processInput(window, verticeVector);
        }

        if (verticesChanged)
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, verticeVector.size() * sizeof(float), verticeVector.data());
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
    static bool mouseHeld = false;
    static bool oneHeld = false;
    double xpos, ypos;
    /* TODO IMPLEMENT METHOD maybe add an enum check method for all of the different textures idk.
    if (glfwGetKey(window, GLFW_KEY_1) && !oneHeld)
    {
        for (int i = 0; i < 6;)
        {
            int idx = i * 5;
            vertices.push_back(baseTile[idx + 0]);               // x
            vertices.push_back(baseTile[idx + 1]);               // y
            vertices.push_back(baseTile[idx + 2]);               // z
            vertices.push_back(baseTile[idx + 3]);
        }
    }
    */
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !mouseHeld)
    {
        
        std::cout << "left click detected" << std::endl;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float ndcX = (xpos / width) * 2.0 - 1.0;
        float ndcY = 1.0 - (ypos / height) * 2.0; // Y is flipped due to opengl origin being bottom left.

        float snappedX = floor(ndcX / blockSize) * blockSize + blockSize / 2.0f;
        float snappedY = floor(ndcY / blockSize) * blockSize + blockSize / 2.0f;

        std::cout << "snappedX: " << snappedX << "snappedY: " << snappedY << std::endl;
        
        UV grassUV = calculateUV(1, 15);
        std::vector<float> baseTile = makeBaseTile(blockSize, grassUV.uMin, grassUV.uMax, grassUV.vMin, grassUV.vMax);

        for (int i = 0; i < 6; i++)
        {
            int idx = i * 5;
            vertices.push_back(baseTile[idx + 0] + snappedX);    // x
            vertices.push_back(baseTile[idx + 1] + snappedY);    // y
            vertices.push_back(baseTile[idx + 2]);               // z
            vertices.push_back(baseTile[idx + 3]);               // u
            vertices.push_back(baseTile[idx + 4]);               // v
        }
        mouseHeld = true;
        return true;
    }
    else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        mouseHeld = false;
    }
    return false;
}
