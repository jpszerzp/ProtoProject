#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

AssimpLoader::AssimpLoader()
{

}

AssimpLoader::~AssimpLoader()
{
	for each (Submesh* sm in m_entries)
	{
		delete sm;
		sm = nullptr;
	}

	m_entries.clear();

	// texture delete handled globally
}

bool AssimpLoader::LoadMeshAssimp(const char* fn)
{
	bool res = false;
	Assimp::Importer importer;

	// absolute path of model obj file
	std::string fnStr(fn);
	std::string modelPath = "Data/Models/" + fnStr;
	const char* mp = modelPath.c_str();

	const aiScene* pScene = importer.ReadFile(mp, aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	if (pScene) 
	{
		res = InitFromAiScene(pScene);
	}
	else 
	{
		DebuggerPrintf("Error parsing '%s': '%s'\n", fn, importer.GetErrorString());
	}

	return res;
}

bool AssimpLoader::InitFromAiScene(const aiScene* pScene)
{
	m_entries.resize(pScene->mNumMeshes);
	m_textures.resize(pScene->mNumMaterials);

	// initialize sub meshes
	for (unsigned int i = 0; i < m_entries.size(); ++i)
	{
		// assimp encapsulates aiMesh for us
		const aiMesh* pMesh = pScene->mMeshes[i];
		InitFromAiMesh(pMesh, i);
	}

	return InitMatFromAiScene(pScene);
}

void AssimpLoader::InitFromAiMesh(const aiMesh* mesh, unsigned int ind, eVertexType type)
{
	MeshBuilder mb;

	m_entries[ind]->m_materialIndex = mesh->mMaterialIndex;

	const aiVector3D zero3(0.0f, 0.0f, 0.0f);

	mb.Begin(DRAW_TRIANGLE, true);
	mb.SetColor(Rgba::WHITE);

	for (unsigned int i = 0 ; i < mesh->mNumVertices ; i++) {
		const aiVector3D* aiPos_0 = &(mesh->mVertices[i]);
		const aiVector3D* aiPos_1 = &(mesh->mVertices[i + 1]);
		const aiVector3D* aiPos_2 = &(mesh->mVertices[i + 2]);

		const aiVector3D* aiNormal_0 = (&(mesh->mNormals[i]) == nullptr ? &zero3 : &(mesh->mNormals[i]));
		const aiVector3D* aiNormal_1 = (&(mesh->mNormals[i + 1]) == nullptr ? &zero3 : &(mesh->mNormals[i + 1]));
		const aiVector3D* aiNormal_2 = (&(mesh->mNormals[i + 2]) == nullptr ? &zero3 : &(mesh->mNormals[i + 2]));

		const aiVector3D* pTexCoord_0 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : &zero3;
		const aiVector3D* pTexCoord_1 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i + 1]) : &zero3;
		const aiVector3D* pTexCoord_2 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i + 2]) : &zero3;

		Vector3 pos_0 = Vector3(aiPos_0->x, aiPos_0->y, aiPos_0->z);
		Vector3 pos_1 = Vector3(aiPos_1->x, aiPos_1->y, aiPos_1->z);
		Vector3 pos_2 = Vector3(aiPos_2->x, aiPos_2->y, aiPos_2->z);

		Vector3 normal_0 = Vector3(aiNormal_0->x, aiNormal_0->y, aiNormal_0->z);
		Vector3 normal_1 = Vector3(aiNormal_1->x, aiNormal_1->y, aiNormal_1->z);
		Vector3 normal_2 = Vector3(aiNormal_2->x, aiNormal_2->y, aiNormal_2->z);

		Vector2 texCoord_0 = Vector2(pTexCoord_0->x, pTexCoord_0->y);
		Vector2 texCoord_1 = Vector2(pTexCoord_1->x, pTexCoord_1->y);
		Vector2 texCoord_2 = Vector2(pTexCoord_2->x, pTexCoord_2->y);

		Vector3 e1 = pos_0 - pos_2;
		Vector3 e2 = pos_1 - pos_2;
		Vector2 uv1 = texCoord_0 - texCoord_2;
		Vector2 uv2 = texCoord_1 - texCoord_2;
		Vector4 tan = mb.CalcTangent(e1, e2, uv1, uv2);
		mb.SetTangent(tan);

		mb.SetUV(texCoord_0);
		mb.SetNormal(normal_0);
		mb.PushVertex(pos_0);

		mb.SetUV(texCoord_1);
		mb.SetNormal(normal_1);
		mb.PushVertex(pos_1);

		mb.SetUV(texCoord_2);
		mb.SetNormal(normal_2);
		mb.PushVertex(pos_2);

		mb.AddTriangle(i, i + 1, i + 2);
	}

	mb.End();

	mb.AdjustSubmesh(m_entries[ind], type, DRAW_TRIANGLE);
	m_entries[ind]->m_immediate = true;
}

bool AssimpLoader::InitMatFromAiScene(const aiScene* pScene)
{
	Renderer* renderer = Renderer::GetInstance();
	bool res = false;

	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		const aiMaterial* pMat = pScene->mMaterials[i];
		m_textures[i] = nullptr;
		if (pMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;

			if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				char* relPath = path.data;
				std::string rpStr(relPath);
				std::string fp = "Data/Images/" + rpStr;
				
				m_textures[i] = renderer->CreateOrGetTexture(fp);

				if (m_textures[i]->GetData() == nullptr)
				{
					DebuggerPrintf("Error loading texture '%s'\n", fp.c_str());
					delete m_textures[i];
					m_textures[i] = nullptr;
					res = false;
				}
			}
		}

		// cannot load texture normally, load default white instead
		if (m_textures[i] == nullptr)
		{
			std::string fp = "Data/Images/white.png";

			m_textures[i] = renderer->CreateOrGetTexture(fp);
			
			res = (m_textures[i]->GetData() != nullptr);
		}
	}

	return res;
}
