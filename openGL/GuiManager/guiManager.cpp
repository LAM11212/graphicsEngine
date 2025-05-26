#include "guiManager.h"

guiManager::guiManager() {}

void guiManager::draw(tileCreator& activeMap, mapManager& mm, float zoom, float blockSize, int windowWidth, int windowHeight, const glm::mat4& proj, GLuint my_tex_id, unsigned int VBO)
{
    ImGui::Begin("Tile Editor");

    ImGui::SliderFloat3("float", &translation.x, 0.0f, 1200.0f);
    ImGui::Checkbox("Show Grid", &gridEnabled);
    ImGui::SameLine();
    if (ImGui::Button("Clear Screen", ImVec2(120, 20)))
    {
        activeMap.clear();
    }
    ImGui::NewLine();
    if (gridEnabled)
    {
        float worldWidth = float(windowWidth) / zoom;
        float worldHeight = float(windowHeight) / zoom;
        activeMap.drawGrid(worldWidth, worldHeight, blockSize, proj);
    }

    drawTileSelector(activeMap, my_tex_id);
    drawMapSelector(mm, activeMap, VBO);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

}

void guiManager::drawTileSelector(tileCreator& activeMap, GLuint my_tex_id)
{
    int columns = 3;
    for (int i = 0; i < activeMap.numOfTileTypes; i++)
    {
        tileCreator::UV uv = activeMap.getTileUVs(i);
        ImVec2 uv0 = ImVec2(uv.uMin, uv.vMax);
        ImVec2 uv1 = ImVec2(uv.uMax, uv.vMin);
        std::string label = "Tile_" + std::to_string(i);
        if (ImGui::ImageButton(label.c_str(), my_tex_id, size, uv0, uv1, bg_color, tint_col))
        {
            activeMap.selectTile(static_cast<tileCreator::tileType>(i));
            std::cout << "Tile " << i << " selected\n";
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Type: %s", activeMap.getStringFromEnum(static_cast<tileCreator::tileType>(i)));
        }
        if ((i + 1) % columns != 0)
        {
            ImGui::SameLine();
        }
    }
    ImGui::NewLine();
}

void guiManager::drawMapSelector(mapManager& mm, tileCreator& activeMap, GLuint VBO)
{
    if (!mm.mapNames.empty() && mm.currentMapIndex < mm.mapNames.size())
    {
        if (ImGui::BeginCombo("Map Selector", mm.mapNames[mm.currentMapIndex].c_str()))
        {
            for (int i = 0; i < mm.mapNames.size(); i++)
            {
                bool selected = (i == mm.currentMapIndex);
                if (ImGui::Selectable(mm.mapNames[i].c_str(), selected))
                {
                    mm.switchTo(i);
                }
            }
            ImGui::EndCombo();
        }
    }
    else
    {
        ImGui::Text("No Maps Available.");
    }

    if (ImGui::Button("Create New Map", ImVec2(120, 20)))
    {
        std::string newName = "Map_" + std::to_string(mm.mapNames.size());
        mm.createMap(newName, VBO);
    }

    if (ImGui::Button("Save", ImVec2(56, 20)))
    {
        activeMap.writeToFile(activeMap.placedTiles, "data.txt");
    }

    ImGui::SameLine();

    if (ImGui::Button("Load", ImVec2(56, 20)))
    {
        activeMap.readFromFile(activeMap.placedTiles, "data.txt");
    }
}