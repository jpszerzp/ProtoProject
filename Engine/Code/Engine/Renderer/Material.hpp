#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Renderer/MaterialPropertyBlock.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector4.hpp"

#include <map>
#include <vector>

class Material
{
public:
	Shader*		m_shader = nullptr;
	std::string m_name;
	bool		m_useLight = false;

	std::map<std::string, PropertyBlock*> m_blocks;
	std::map<int, Texture*> m_textures;

	// MULTI-PASS
	ShaderChannel* m_channel = nullptr;

	// This is single property approach, different from property block approach.
	// Replaced by property block - no global uniforms for materials
	//std::vector<Property*> m_singleProps;

public:
	Material();
	~Material();

	static Material* AcquireShaderResource(const char* fp);
	static Material* AcquireShaderChannelResource(const char* fp);

	void FillPropertyBlocks();
	void FillTextures(bool isFont = false);
	//void FillSeparateProps();

	// Set property in property block
	template<typename T>
	void SetProperty(const char* name, T const &v);
	void SetProperty(const char* name, void const* data, size_t datasize);

	// Non-templates for single props
	//void SetSingleProperty(const char* name, const Rgba& v);
	//void SetSingleProperty(const char* name, const Vector4& v);
	void RemoveProperty(const char* name);

	void SetTexture(int bindIdx, const char* bindName, const char* fp);

	Material* Clone() const;

	PropertyBlock* CreateOrGetBlock(sPropertyBlockInfo* info);

	bool UseLight() const { return m_useLight; }
};

template<typename T>
void Material::SetProperty(const char* name, T const &value)
{
	SetProperty(name, &value, sizeof(T));
}