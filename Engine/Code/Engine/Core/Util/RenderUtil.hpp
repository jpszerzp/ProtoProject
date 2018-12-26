#pragma once

#include "Engine/Renderer/Renderer.hpp"

extern void DrawGraphAlpha(Renderer* renderer, Mesh* mesh);
extern void DrawTextCut(Mesh* textMesh);
extern void DrawTexts(const std::vector<Mesh*>& meshes);
extern void DrawPoint(Mesh* point_mesh, uint size = 10.f);
extern void DrawLine(Mesh* line_mesh);
extern void DrawTriangle(Mesh* triangle_mesh);
extern void DrawTetrahedron(Mesh* tetra_mesh);