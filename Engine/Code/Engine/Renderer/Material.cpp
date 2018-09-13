#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MaterialProperty.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Material::Material()
{

}


Material::~Material()
{
	for (std::map<std::string, PropertyBlock*>::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it)
	{
		PropertyBlock* block = it->second;
		
		if (block != nullptr)
		{
			//delete block;
			block = nullptr;
		}
	}

	m_blocks.clear();
}


void Material::SetProperty(const char* name, void const* data, size_t dataSize)
{
	ShaderProgramInfo* shaderInfo = m_shader->GetProgramInfo();

	// Find property with name
	sPropertyInfo* propInfo = shaderInfo->FindInfo(name);	

	// Find block that property belongs to
	sPropertyBlockInfo* blockInfo = propInfo->GetOwningBlock();

	// Fill property block (UBO) with info
	PropertyBlock* block = CreateOrGetBlock(blockInfo);

	ASSERT_RECOVERABLE(propInfo->size == dataSize, "Setting of property does not match in size.");

	// Copy to GPU
	block->UpdateCPU(propInfo->offset, data, dataSize);
}

void Material::SetTexture(int bindIdx, const char*, const char* fp)
{
	Renderer* renderer = Renderer::GetInstance();

	Sampler* sampler = new Sampler();
	Texture* texture = renderer->CreateOrGetTexture(fp);
	texture->SetSampler(sampler);

	renderer->SetTexture2D(bindIdx, texture);
	renderer->SetSampler2D(bindIdx, sampler);

	m_textures.emplace(bindIdx, texture);
}

Material* Material::Clone() const
{
	Material* cloned = new Material();
	
	cloned->m_shader = m_shader;
	cloned->m_channel = m_channel;
	cloned->m_name = m_name;
	cloned->m_useLight = m_useLight;
	cloned->m_blocks = m_blocks;
	cloned->m_textures = m_textures;
	//cloned->m_singleProps = m_singleProps;

	return cloned;
}

Material* Material::AcquireShaderResource(const char* fp)
{
	Renderer* r = Renderer::GetInstance();

	Material* mat = new Material();

	XMLDocument matDoc;
	matDoc.LoadFile(fp);

	// material name/id in .mat
	XMLElement* idEle = matDoc.FirstChildElement();
	mat->m_name = ParseXmlAttribute(*idEle, "id", "");

	for (XMLElement* ele = idEle->FirstChildElement();
		ele != 0; ele = ele->NextSiblingElement())
	{
		std::string prop = ele->Value();
		if (prop == "shader")
		{
			// shader id specified in .mat
			std::string shaderName = ParseXmlAttribute(*ele, "id", "");

			// clone original shader
			Shader* shader = r->CreateOrGetShader(shaderName);

			mat->m_shader = shader;
		}
		// adjust of default properties from .mat
		// adjust only on cloned instance shader
		else if (prop == "texture")
		{
			int bindIdx = ParseXmlAttribute(*ele, "bind", -1);
			std::string bindName = ParseXmlAttribute(*ele, "name", "");
			std::string bindSrc = ParseXmlAttribute(*ele, "value", "");

			ShaderProgramInfo* pInfo = mat->m_shader->GetProgramInfo();
			sShaderTexture* tex = pInfo->FindDefaultTex(bindIdx);
			
			if (tex != nullptr)
			{
				// name would the same as bind index is the same
				tex->m_bindSrc = bindSrc;
			}
			else
			{
				tex = new sShaderTexture(bindIdx, bindName, bindSrc);
				pInfo->m_defaultTextures.push_back(tex);
			}
		}
		else if (prop == "color")
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			ShaderProgramInfo* pInfo = mat->m_shader->GetProgramInfo();
			sShaderProperty* sprop = pInfo->FindDefaultProperty(name);

			if (sprop != nullptr)
			{
				// name is the same
				sprop->m_bindValue = value;
			}
			else
			{
				sprop = new sShaderProperty(type, name, value);
				pInfo->m_defaultResources.push_back(sprop);
			}
		}
		else if (prop == "float")
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			ShaderProgramInfo* pInfo = mat->m_shader->GetProgramInfo();
			sShaderProperty* sprop = pInfo->FindDefaultProperty(name);

			if (sprop != nullptr)
			{
				// type is not changing either
				sprop->m_bindValue = value;
			}
			else 
			{
				sprop = new sShaderProperty(type, name, value);
				pInfo->m_defaultResources.push_back(sprop);
			}
		}
	}

	return mat;
}


Material* Material::AcquireShaderChannelResource(const char* fp)
{
	Renderer* r = Renderer::GetInstance();

	Material* mat = new Material();

	XMLDocument matDoc;
	matDoc.LoadFile(fp);

	// material name/id in .mat
	XMLElement* idEle = matDoc.FirstChildElement();
	mat->m_name = ParseXmlAttribute(*idEle, "id", "");

	for (XMLElement* ele = idEle->FirstChildElement();
		ele != 0; ele = ele->NextSiblingElement())
	{
		// .mat
		std::string prop = ele->Value();
		if (prop == "shader")
		{
			// shader id specified in .mat
			std::string shaderName = ParseXmlAttribute(*ele, "id", "");

			// clone original shader
			ShaderChannel* channel = r->CreateOrGetShaderChannel(shaderName);

			mat->m_channel = channel;
		}
		else if (prop == "texture")
		{
			int bindIdx = ParseXmlAttribute(*ele, "bind", -1);
			std::string bindName = ParseXmlAttribute(*ele, "name", "");
			std::string bindSrc = ParseXmlAttribute(*ele, "value", "");

			for (int passIdx = 0; passIdx < ShaderChannel::MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = mat->m_channel->m_shaderPasses[passIdx];

				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					sShaderTexture* stex = pInfo->FindDefaultTex(bindIdx);

					if (stex != nullptr)
					{
						stex->m_bindSrc = bindSrc;
					}
					else
					{
						stex = new sShaderTexture(bindIdx, bindName, bindSrc);
						pInfo->m_defaultTextures.push_back(stex);
					}
				}
			}
		}
		else if (prop == "color")
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			for (int passIdx = 0; passIdx < ShaderChannel::MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = mat->m_channel->m_shaderPasses[passIdx];

				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					sShaderProperty* sprop = pInfo->FindDefaultProperty(name);

					if (sprop != nullptr)
					{
						sprop->m_bindValue = value;
					}
					else
					{
						sprop = new sShaderProperty(type, name, value);
						pInfo->m_defaultResources.push_back(sprop);
					}
				}
			}
		}
		else if (prop == "float")
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			for (int passIdx = 0; passIdx < ShaderChannel::MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = mat->m_channel->m_shaderPasses[passIdx];

				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					sShaderProperty* sprop = pInfo->FindDefaultProperty(name);

					if (sprop != nullptr)
					{
						sprop->m_bindValue = value;
					}
					else
					{
						sprop = new sShaderProperty(type, name, value);
						pInfo->m_defaultResources.push_back(sprop);
					}
				}
			}
		}
	}

	return mat;
}


PropertyBlock* Material::CreateOrGetBlock(sPropertyBlockInfo* info)
{
	PropertyBlock* propBlock = m_blocks[info->blockName];

	if(propBlock != nullptr)
	{
		return propBlock;
	}
	else
	{
		propBlock =  new PropertyBlock();
		propBlock->m_blockInfo = info;

		m_blocks.emplace(info->blockName, propBlock);
		
		return propBlock;
	}
}


void Material::FillPropertyBlocks()
{
	ShaderProgramInfo* pInfo = m_shader->GetProgramInfo();
	std::vector<sPropertyBlockInfo*>& binfo = pInfo->m_blockInfos;

	// stuff blocks with info but empty data
	for each (sPropertyBlockInfo* info in binfo)
	{
		PropertyBlock* block = new PropertyBlock();

		block->m_blockInfo = info;

		char* data = new char[info->blockSize]();
		block->m_dataBlock = data;

		m_blocks.emplace(info->blockName, block);
	}

	// actually fill initial data given default shader property
	for each (sShaderProperty* prop in pInfo->m_defaultResources)
	{
		std::string name = prop->m_bindName;
		std::string type = prop->m_type;
		std::string value = prop->m_bindValue;

		if (type == "color")
		{
			Rgba color;
			color.SetFromText(value.c_str());

			Vector4 colorV4;
			color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);

			SetProperty(name.c_str(), colorV4);
		}
		else if (type == "float")
		{
			float val = (float)(atof(value.c_str()));

			SetProperty(name.c_str(), val);
		}
	}
}


void Material::FillTextures(bool isFont)
{
	ShaderProgramInfo* pInfo = m_shader->GetProgramInfo();
	std::vector<sShaderTexture*>& textures = pInfo->m_defaultTextures;

	for each (sShaderTexture* tex in textures)
	{
		int idx = tex->m_bindIdx;

		std::string bindName = tex->m_bindName;
		std::string bindSrc = tex->m_bindSrc;

		std::string fullPath = "";
		if (!isFont)
		{
			fullPath = "Data/Images/" + bindSrc;
		}
		else
		{
			fullPath = "Data/Fonts/" + bindSrc;
		}

		Renderer* r = Renderer::GetInstance();
		Texture* texture = r->CreateOrGetTexture(fullPath);
		Sampler* sampler = new Sampler();
		texture->SetSampler(sampler);

		m_textures.emplace(idx, texture);
	}
}


// deprecated
/*
void Material::FillSeparateProps()
{
	ShaderProgramInfo* pInfo = m_shader->GetProgramInfo();
	std::vector<sShaderProperty*>& props = pInfo->m_defaultResources;

	for each (sShaderProperty* prop in props)
	{
		if (prop->m_type == "color")
		{
			std::string name = prop->m_bindName;
			std::string value = prop->m_bindValue;

			Rgba color;
			color.SetFromText(value.c_str());

			Vector4 colorV4;
			color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);

			PropertyVec4* singleProp = new PropertyVec4(name.c_str(), colorV4);
			
			m_singleProps.push_back(singleProp);
		}
	}
}
*/

// deprecated
/*
void Material::SetSingleProperty(const char* name, const Rgba& v)
{
	Vector4 colorV4;
	v.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);

	SetSingleProperty(name, colorV4);
}
*/


// deprecated
/*
void Material::SetSingleProperty(const char* name, const Vector4& v)
{
	RemoveProperty(name);
	PropertyVec4* prop = new PropertyVec4(name, v);
	m_singleProps.push_back(prop);
}
*/


void Material::RemoveProperty(const char*)
{
	// Remove prop that is already in single props vector
	// - deprecated (prop block is used)

}
