#pragma once

#include "Engine/Renderer/MaterialPropertyBlock.hpp"

struct sShaderProperty
{
	std::string m_type;
	std::string m_bindName;
	std::string m_bindValue;

	sShaderProperty(std::string type, std::string name, std::string value);
	~sShaderProperty();
};

struct sShaderTexture
{
	int m_bindIdx;
	std::string m_bindName;
	std::string m_bindSrc;

	sShaderTexture(int idx, std::string name, std::string src);
	~sShaderTexture();
};

class ShaderProgramInfo
{
public:
	ShaderProgramInfo() {}
	~ShaderProgramInfo() {}

	// property block (UBO)
	const sPropertyBlockInfo* FindBlockInfo( const char* blockname ); 
	const sPropertyBlockInfo* FindContainingBlock( const char* propname ); 
	sPropertyInfo* FindInfo( const char* name ); 

	// shader property
	sShaderProperty* FindDefaultProperty( std::string name );

	// tex
	sShaderTexture* FindDefaultTex( int bindIdx );

	void Clear();

public:
	std::vector<sPropertyBlockInfo*> m_blockInfos; 

	// defaults directly from shader.xml
	std::vector<sShaderProperty*>	 m_defaultResources;
	std::vector<sShaderTexture*>	 m_defaultTextures;
};