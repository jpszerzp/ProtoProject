#include "Engine/Core/Util/RenderUtil.hpp"

void DrawGraphAlpha(Renderer* renderer, Mesh* mesh)
{
	if (mesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("2d_direct_alpha");
		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(mesh);
	}
}

void DrawTextCut(Mesh* textMesh)
{
	Renderer* renderer = Renderer::GetInstance();

	if (textMesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("cutout_nonmodel");
		Texture* texture = renderer->CreateOrGetTexture("Data/Fonts/SquirrelFixedFont.png");
		renderer->UseShader(shader);
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		renderer->DrawMesh(textMesh);
	}
}

void DrawTexts(const std::vector<Mesh*>& meshes)
{
	for (Mesh* mesh : meshes)
		DrawTextCut(mesh);
}

extern void DrawPoint(Mesh* point_mesh)
{
	Renderer* renderer = Renderer::GetInstance();
	if (point_mesh != nullptr)
	{
		Shader* shader = renderer->CreateOrGetShader("wireframe_color");
		renderer->UseShader(shader);

		Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());
		glPointSize(10.f);

		renderer->m_objectData.model = Matrix44::IDENTITY;

		renderer->DrawMesh(point_mesh);
	}
}
