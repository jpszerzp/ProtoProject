#include "Engine/Renderer/ShaderProperties.hpp"

#include <algorithm>

sShaderProperty::sShaderProperty(std::string type, std::string name, std::string value)
{
	m_type = type;
	m_bindName = name;
	m_bindValue = value;
}


sShaderProperty::~sShaderProperty()
{

}


sShaderTexture::sShaderTexture(int idx, std::string name, std::string src)
{
	m_bindIdx = idx;
	m_bindName = name;
	m_bindSrc = src;
}


sShaderTexture::~sShaderTexture()
{

}


void ShaderProgramInfo::Clear()
{
	m_blockInfos.clear();
}


sPropertyInfo* ShaderProgramInfo::FindInfo(const char* name)
{
	sPropertyInfo* propInfo = nullptr;

	for (std::vector<sPropertyBlockInfo*>::size_type blockIdx = 0; 
		blockIdx < m_blockInfos.size(); ++blockIdx)
	{
		sPropertyBlockInfo* blockInfo = m_blockInfos[blockIdx];
		
		for each(sPropertyInfo* info in blockInfo->properties)
		{
			if (info->name == name)
			{
				propInfo = info;
			}
		}
	}

	return propInfo;
}


sShaderProperty* ShaderProgramInfo::FindDefaultProperty( std::string name )
{
	sShaderProperty* defaultProperty = nullptr;

	for each (sShaderProperty* property in m_defaultResources)
	{
		if (property->m_bindName == name)
		{
			defaultProperty = property;
		}
	}

	return defaultProperty;
}


sShaderTexture* ShaderProgramInfo::FindDefaultTex( int bindIdx )
{
	sShaderTexture* defaultTexture = nullptr;

	for each (sShaderTexture* tex in m_defaultTextures)
	{
		if (tex->m_bindIdx == bindIdx)
		{
			defaultTexture = tex;
		}
	}

	return defaultTexture;
}
