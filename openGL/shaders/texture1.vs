#version 400 core

layout (location = 0) in vec4 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 projection;

void main() {

    gl_Position = projection * aPos;
    texCoord = aTexCoord;
}