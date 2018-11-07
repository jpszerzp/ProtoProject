#pragma once
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Submesh.hpp"
#include "Engine/Core/Vertex.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <vector>
#include <set>

class AssimpLoader
{
public:
	AssimpLoader();
	AssimpLoader(const char* path);
	~AssimpLoader();

	// assimp mesh load
	bool LoadModel(const char* fn);

	std::vector<Mesh*>& GetEntries() { return m_meshes; }

private:
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh* ProessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture*> LoadMaterialTexture(aiMaterial* mat, aiTextureType type);

public:
	std::vector<Mesh*> m_meshes;
	std::string m_path;
	std::set<Vector3> m_vertPos;
};