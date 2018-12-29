#include "Engine/Core/AssimpLoader.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"

AssimpLoader::AssimpLoader()
{

}

AssimpLoader::AssimpLoader(const char* path)
{
	if (!LoadModel(path))
		ASSERT_RECOVERABLE(false, "Failed to import model with assimp");
}

AssimpLoader::~AssimpLoader()
{
	for each (Mesh* sm in m_meshes)
	{
		delete sm;
		sm = nullptr;
	}

	m_meshes.clear();

	// texture delete handled globally
}

bool AssimpLoader::LoadModel(const char* fn)
{
	bool res = false;
	Assimp::Importer importer;

	// absolute path of model obj file
	std::string fnStr(fn);
	std::string modelPath = "Data/Models/" + fnStr;
	const char* mp = modelPath.c_str();

	const aiScene* pScene = importer.ReadFile(mp, aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
		aiProcess_GenUVCoords | aiProcess_CalcTangentSpace);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		DebuggerPrintf("Error parsing '%s': '%s'\n", fn, importer.GetErrorString());
	else
	{
		ProcessNode(pScene->mRootNode, pScene);
		res = true;
	}

	return res;
}

void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (uint i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(ProessMesh(mesh, scene));
	}

	for (uint i = 0; i < node->mNumChildren; ++i)
		ProcessNode(node->mChildren[i], scene);
}

Mesh* AssimpLoader::ProessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<sVertexBuilder> vertices;
	std::vector<uint> indices;
	//std::vector<Texture*> textures;

	for (uint i = 0; i < mesh->mNumVertices; ++i)
	{
		// vert pos
		sVertexBuilder vertex;
		Vector3 pos;
		pos.x = mesh->mVertices[i].x;
		pos.y = mesh->mVertices[i].y;
		pos.z = mesh->mVertices[i].z;
		vertex.m_position = pos;

		// normal
		Vector3 n;
		n.x = mesh->mNormals[i].x;
		n.y = mesh->mNormals[i].y;
		n.z = mesh->mNormals[i].z;
		vertex.m_normal = n;

		// color
		if (mesh->HasVertexColors(0))
		{
			Rgba color;
			color.r = (unsigned char)mesh->mColors[0]->r;
			color.g = (unsigned char)mesh->mColors[0]->g;
			color.b = (unsigned char)mesh->mColors[0]->b;
			color.a = (unsigned char)mesh->mColors[0]->a;
			vertex.m_color = color;
		}
		else
			vertex.m_color = Rgba();		// 255, 255, 255, 255

		if (mesh->mTextureCoords[0])
		{
			Vector2 uv;
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
			vertex.m_uv = uv;
		}
		else
			vertex.m_uv = Vector2(0.f, 0.f);

		// tangent
		Vector3 tangent;
		tangent.x = mesh->mTangents[i].x;
		tangent.y = mesh->mTangents[i].y;
		tangent.z = mesh->mTangents[i].z;
		vertex.m_tangent = Vector4(tangent, 1.f);

		// no bitangent either for vertex builder

		vertices.push_back(vertex);

		m_vertPos.emplace(vertex.m_position);
	}

	for (uint i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (uint j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}

	// create mesh
	Mesh* theMesh = new Mesh();

	// optional: directly read texture from material file
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture*> diffuse = LoadMaterialTexture(mat, aiTextureType_DIFFUSE);
	std::vector<Texture*> specular = LoadMaterialTexture(mat, aiTextureType_SPECULAR);
	std::vector<Texture*> normal = LoadMaterialTexture(mat, aiTextureType_NORMALS);
	std::vector<Texture*> height = LoadMaterialTexture(mat, aiTextureType_HEIGHT);
	theMesh->m_texFromAssimp.insert(theMesh->m_texFromAssimp.end(), diffuse.begin(), diffuse.end());
	theMesh->m_texFromAssimp.insert(theMesh->m_texFromAssimp.end(), specular.begin(), specular.end());
	theMesh->m_texFromAssimp.insert(theMesh->m_texFromAssimp.end(), normal.begin(), normal.end());
	theMesh->m_texFromAssimp.insert(theMesh->m_texFromAssimp.end(), height.begin(), height.end());

	// configure draw call
	eDrawPrimitiveType type = DRAW_TRIANGLE;		// we have enabled assimp triangulation
	uint startIdx = 0;
	uint usingIdx = true;			// we use IBO by default
	uint eleCount = usingIdx ? (uint)(indices.size()) : (uint)(vertices.size());
	theMesh->SetDrawInstruction(type, usingIdx, startIdx, eleCount);
	
	// our type is vert pcu 3d
	theMesh->SetLayout(Vertex_3DPCU::s_layout);
	
	// given vertices and indices, pass them to GPU
	size_t vsize = theMesh->m_layout.m_stride * vertices.size();
	size_t isize = sizeof(uint) * indices.size();
	std::vector<Vertex_3DPCU> pcu;
	for (std::vector<sVertexBuilder>::size_type vbCount = 0; vbCount < vertices.size(); ++vbCount)
	{
		Vertex_3DPCU vt(vertices[vbCount]);
		pcu.push_back(vt);
	}
	theMesh->m_vbo.CopyToGPU(vsize, &pcu[0]);
	theMesh->m_ibo.CopyToGPU(isize, &indices[0]);
	
	// set buffer stride and count for book keeping
	theMesh->SetVertices((uint)(vertices.size()), sizeof(Vertex_3DPCU));
	theMesh->SetIndices((uint)(indices.size()), sizeof(uint));

	return theMesh;
}

/* This could be replaced by material system in this engine */
std::vector<Texture*> AssimpLoader::LoadMaterialTexture(aiMaterial* mat, aiTextureType type)
{
	Renderer* renderer = Renderer::GetInstance();
	std::vector<Texture*> textures;
	for (uint i = 0; i < mat->GetTextureCount(type); ++i)
	{
		// this will not be called if material file does not specify texture info
		// Or, there is no such material file at all
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string dir = "Data/Images/";
		std::string subDir(str.C_Str());
		dir += subDir;
		//Texture* texture = new Texture(dir);
		Texture* texture = renderer->CreateOrGetTexture(dir);
		textures.push_back(texture);
	}
	return textures;
}
