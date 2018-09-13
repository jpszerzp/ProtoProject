#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Submesh.hpp"
#include "Engine/Core/Vertex.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <vector>

class AssimpLoader
{
public:
	AssimpLoader();
	~AssimpLoader();

	// assimp mesh load
	bool LoadMeshAssimp(const char* fn);

	std::vector<Submesh*>& GetEntries() { return m_entries; }
	std::vector<Texture*>& GetTextures() { return m_textures; }

private:
	bool InitFromAiScene(const aiScene* pScene);
	void InitFromAiMesh(const aiMesh* mesh, unsigned int ind, eVertexType type = VERT_PCU);
	bool InitMatFromAiScene(const aiScene* pScene);

private:
	std::vector<Submesh*> m_entries;
	std::vector<Texture*> m_textures;
};