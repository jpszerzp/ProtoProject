#pragma once

#include "Engine/Renderer/Renderer.hpp"

extern void DrawGraphAlpha(Renderer* renderer, Mesh* mesh);
extern void DrawTextCut(Mesh* textMesh);
extern void DrawTexts(const std::vector<Mesh*>& meshes);