#pragma once
#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../tileEditor/tileCreator.h"
#include "../mapManager/mapManager.h"
#include <GLFW/glfw3.h>

class guiManager
{
public:
	guiManager();

	void draw(tileCreator& activeMap, mapManager& mm, float zoom, float blockSize, int windowWidth, int windowHeight, const glm::mat4& proj, GLuint my_tex_id, unsigned int VBO);
	bool gridEnabled;
	glm::vec3 translation = { 0.0f, 0.0f, 0.0f };

	ImVec2 size = ImVec2(32.0f, 32.0f);
	ImVec4 bg_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

private:
	void drawTileSelector(tileCreator& activeMap, GLuint my_tex_id);
	void drawMapSelector(mapManager& mm, tileCreator& activeMap, GLuint VBO);
};
#endif;