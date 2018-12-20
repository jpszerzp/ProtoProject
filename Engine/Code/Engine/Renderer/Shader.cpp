#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Util/XmlUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static eRenderQueue ToRenderQueue(std::string queueStr)
{
	if (queueStr == "opague")
	{
		return RENDER_QUEUE_OPAGUE;
	}
	else if (queueStr == "alpha")
	{
		return RENDER_QUEUE_ALPHA;
	}
	else
	{
		return NUM_OF_RENDER_QUEUE;
	}
}

Shader::Shader()
{
	// default layer and queue
	m_layer = 0;
	m_queue = RENDER_QUEUE_OPAGUE;
}


Shader::~Shader()
{
	delete m_program;
	m_program = nullptr;

	delete m_programInfo;
	m_programInfo = nullptr;
}


Shader* Shader::AcquireResource(const char* fp)
{
	Shader* res = new Shader();

	XMLDocument shaderDoc;
	shaderDoc.LoadFile(fp);

	for (XMLElement* ele = shaderDoc.FirstChildElement()->FirstChildElement();
		ele != 0; ele = ele->NextSiblingElement())
	{
		std::string prop = ele->Value();
		if (prop.compare("program") == 0)
		{
			std::string vsPath;
			std::string fsPath;
			ShaderProgram* shaderProgram = new ShaderProgram();
			ShaderProgramInfo* shaderProgramInfo = new ShaderProgramInfo();

			for (XMLElement* spHeader = ele->FirstChildElement(); 
				spHeader != 0; spHeader = spHeader->NextSiblingElement())
			{
				std::string spHeaderStr = spHeader->Value();
				if (spHeaderStr.compare("vertex") == 0)
				{
					vsPath = ParseXmlAttribute(*spHeader, "file", "");
				}
				else if (spHeaderStr.compare("fragment") == 0)
				{
					fsPath = ParseXmlAttribute(*spHeader, "file", "");
				}
			}

			vsPath = "Data/" + vsPath;
			fsPath = "Data/" + fsPath;
			bool programLinked = shaderProgram->LoadFromFiles(vsPath.c_str(), fsPath.c_str(), "");
			if (!programLinked)
			{
				TODO("Load the invalid shader program");
				res->m_program = nullptr;
			}
			else
			{
				res->m_program = shaderProgram;
				res->m_programInfo = shaderProgramInfo;

				// Fill property block
				Renderer* r = Renderer::GetInstance();
				GLuint handle = shaderProgram->GetHandle();
				r->FillInfo(shaderProgramInfo, handle);
			}
		}
		else if (prop.compare("order") == 0)
		{
			int layerNum = ParseXmlAttribute(*ele, "layer", 0);		
		
			std::string queueStr = ParseXmlAttribute(*ele, "queue", "opague");
			eRenderQueue queue = ToRenderQueue(queueStr);

			res->m_layer = layerNum;
			res->m_queue = queue;
		}
		else if (prop.compare("blend") == 0)
		{
			for (XMLElement* header = ele->FirstChildElement(); 
				header != 0; header = header->NextSiblingElement())
			{
				std::string opAttr = ParseXmlAttribute(*header, "op", "");
				std::string srcAttr = ParseXmlAttribute(*header, "src", "");
				std::string destAttr = ParseXmlAttribute(*header, "dest", "");

				std::string blendProp = header->Value();
				if (blendProp.compare("color") == 0)
				{
					// operation
					if (opAttr == "add")
					{
						res->m_state.m_colorBlendOp = COMPARE_ADD;
					}
					
					// source
					if (srcAttr == "blend_one")
					{
						res->m_state.m_colorSrcFactor = BLEND_ONE;
					}
					else if (srcAttr == "blend_src_alpha")
					{
						res->m_state.m_colorSrcFactor = BLEND_SRC_ALPHA;
					}

					// destination
					if (destAttr == "blend_zero")
					{
						res->m_state.m_colorDstFactor = BLEND_ZERO;
					}
					else if (destAttr == "blend_one_minus_src_alpha")
					{
						res->m_state.m_colorDstFactor = BLEND_ONE_MINUS_SRC_ALPHA;
					}
				}
				else if (blendProp.compare("alpha") == 0)
				{
					// operation
					if (opAttr == "add")
					{
						res->m_state.m_alphaBlendOp = COMPARE_ADD;
					}
					
					// source
					if (srcAttr == "blend_src_alpha")
					{
						res->m_state.m_alphaSrcFactor = BLEND_SRC_ALPHA;
					}
					else if (srcAttr == "blend_one")
					{
						res->m_state.m_alphaSrcFactor = BLEND_ONE;
					}

					// destination
					if (destAttr == "blend_one_minus_src_alpha")
					{
						res->m_state.m_alphaDstFactor = BLEND_ONE_MINUS_SRC_ALPHA;
					}
					else if (destAttr == "blend_one")
					{
						res->m_state.m_alphaDstFactor = BLEND_ONE;
					}
				}
			}
		}
		else if (prop.compare("depth") == 0)
		{
			std::string compareStr = ParseXmlAttribute(*ele, "compare", "");

			size_t pos = compareStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueDepthCompareStr = compareStr.substr(0, pos);
				eDepthCompare trueDepthCompare = SelectCompare(trueDepthCompareStr);
				if (trueDepthCompare != NUM_COMPARE_MODE)
				{
					res->m_state.m_depthCompares.push_back(trueDepthCompare);
				}

				compareStr = compareStr.substr(pos + 1);
				pos = compareStr.find_first_of("|");
			}
			eDepthCompare compare = SelectCompare(compareStr);
			if (compare != NUM_COMPARE_MODE)
			{
				res->m_state.m_depthCompares.push_back(compare);
			}

			bool write = ParseXmlAttribute(*ele, "write", true);
			res->m_state.m_depthWrite = write;
		}
		else if (prop.compare("fill") == 0)
		{
			std::string modeStr = ParseXmlAttribute(*ele, "mode", "");
			if (modeStr.compare("wireframe") == 0)
			{
				res->m_state.m_fillMode = FILLMODE_WIRE;
			}
		}
		else if (prop.compare("wind") == 0)
		{
			/*
			std::string orderStr = ParseXmlAttribute(*ele, "order", "");
			if (orderStr.compare("ccw") == 0)
			{
				res->m_state.m_windOrder = WIND_COUNTER_CLOCKWISE;
			}
			*/

			std::string orderStr = ParseXmlAttribute(*ele, "order", "");

			size_t pos = orderStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueOrderStr = orderStr.substr(0, pos);
				eWindOrder trueOrder = SelectOrder(trueOrderStr);
				if (trueOrder != NUM_WIND_ORDER)
				{
					res->m_state.m_windOrders.push_back(trueOrder);
				}

				orderStr = orderStr.substr(pos + 1);
				pos = orderStr.find_first_of("|");
			}
			eWindOrder order = SelectOrder(orderStr);
			if (order != NUM_WIND_ORDER)
			{
				res->m_state.m_windOrders.push_back(order);
			}
		}
		else if (prop.compare("cull") == 0)
		{
			std::string faceStr = ParseXmlAttribute(*ele, "face", "");

			size_t pos = faceStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueFaceStr = faceStr.substr(0, pos);
				eCullMode trueCullMode = SelectCull(trueFaceStr);
				if (trueCullMode != NUM_CULL_MODE)
				{
					res->m_state.m_cullModes.push_back(trueCullMode);
				}

				faceStr = faceStr.substr(pos + 1);
				pos = faceStr.find_first_of("|");
			}
			eCullMode cull = SelectCull(faceStr);
			if (cull != NUM_CULL_MODE)
			{
				res->m_state.m_cullModes.push_back(cull);
			}
		}
		// Defaults
		else if (prop.compare("texture") == 0)
		{
			int idx = ParseXmlAttribute(*ele, "bind", -1);
			std::string name = ParseXmlAttribute(*ele, "name", "");
			std::string src = ParseXmlAttribute(*ele, "value", "");

			sShaderTexture* st = new sShaderTexture(idx, name, src);

			ShaderProgramInfo* pInfo = res->m_programInfo;
			pInfo->m_defaultTextures.push_back(st);
		}
		else if (prop.compare("color") == 0)
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			sShaderProperty* sp = new sShaderProperty(type, name, value);

			ShaderProgramInfo* pInfo = res->m_programInfo;
			pInfo->m_defaultResources.push_back(sp);
		}
		else if (prop.compare("float") == 0)
		{
			// save as color due to structure
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			sShaderProperty* sp = new sShaderProperty(type, name, type);

			ShaderProgramInfo* pInfo = res->m_programInfo;
			pInfo->m_defaultResources.push_back(sp);
		}
	}

	return res;
}


Shader* Shader::MakeShader(const char* fp)
{
	Shader* res = new Shader();

	XMLDocument shaderDoc;
	shaderDoc.LoadFile(fp);

	for (XMLElement* ele = shaderDoc.FirstChildElement()->FirstChildElement(); ele != 0; ele = ele->NextSiblingElement())
	{
		std::string prop = ele->Value();
		if (prop.compare("program") == 0)
		{
			std::string vsPath;
			std::string fsPath;
			ShaderProgram* shaderProgram = new ShaderProgram();
			//ShaderProgramInfo* shaderProgramInfo = new ShaderProgramInfo();

			for (XMLElement* spHeader = ele->FirstChildElement(); 
				spHeader != 0; spHeader = spHeader->NextSiblingElement())
			{
				std::string spHeaderStr = spHeader->Value();
				if (spHeaderStr.compare("vertex") == 0)
				{
					vsPath = ParseXmlAttribute(*spHeader, "file", "");
				}
				else if (spHeaderStr.compare("fragment") == 0)
				{
					fsPath = ParseXmlAttribute(*spHeader, "file", "");
				}
			}

			vsPath = "Data/" + vsPath;
			fsPath = "Data/" + fsPath;
			bool programLinked = shaderProgram->LoadFromFiles(vsPath.c_str(), fsPath.c_str(), "");
			if (!programLinked)
			{
				TODO("Load the invalid shader program");
				res->m_program = nullptr;
			}
			else
				res->m_program = shaderProgram;
		}
	}

	return res;
}

void Shader::SetDepth(eDepthCompare compare, bool write)
{
	m_state.m_depthWrite = write;
	m_state.m_depthCompare = compare;
}


void Shader::SetCull( eCullMode cull )
{
	m_state.m_cullMode = cull;
}


void Shader::SetFill( eFillMode fill )
{
	m_state.m_fillMode = fill;
}


void Shader::SetWind(eWindOrder wind)
{
	m_state.m_windOrder = wind;
}


eDepthCompare Shader::SelectCompare(std::string compareStr)
{
	eDepthCompare res = NUM_COMPARE_MODE;

	if (compareStr.compare("less") == 0)
	{
		res = COMPARE_LESS;
	}
	else if (compareStr.compare("always") == 0)
	{
		res = COMPARE_ALWAYS;
	}
	else if (compareStr.compare("lequal") == 0)
	{
		res = COMPARE_LEQUAL;
	}
	else if (compareStr.compare("never") == 0)
	{
		res = COMPARE_NEVER;
	}
	else if (compareStr.compare("equal") == 0)
	{
		res = COMPARE_EQUAL;
	}
	else if (compareStr.compare("gequal") == 0)
	{
		res = COMPARE_GEQUAL;
	}
	else if (compareStr.compare("nequal") == 0)
	{
		res = COMPARE_NOT_EQUAL;
	}
	else if (compareStr.compare("greater") == 0)
	{
		res = COMPARE_GREATER;
	}

	return res;
}

eCullMode Shader::SelectCull(std::string cullStr)
{
	eCullMode res = NUM_CULL_MODE;

	if (cullStr.compare("front") == 0)
	{
		res = CULLMODE_FRONT;
	}
	else if (cullStr.compare("back") == 0)
	{
		res = CULLMODE_BACK;
	}
	else if (cullStr.compare("none") == 0)
	{
		res = CULLMODE_NONE;
	}

	return res;
}

eWindOrder Shader::SelectOrder(std::string orderStr)
{
	eWindOrder res = NUM_WIND_ORDER;

	if (orderStr.compare("ccw") == 0)
	{
		res = WIND_COUNTER_CLOCKWISE;
	}
	else if (orderStr.compare("cw") == 0)
	{
		res = WIND_CLOCKWISE;
	}

	return res;
}

ShaderChannel* ShaderChannel::AcquireResource(const char* fp)
{
	ShaderChannel* res = new ShaderChannel();

	// passes start with null pointers
	for (int passIdx = 0; passIdx < MAX_SHADER_PASS; ++passIdx)
	{
		res->m_shaderPasses[passIdx] = nullptr;
	}

	XMLDocument channelDoc;
	channelDoc.LoadFile(fp);

	for (XMLElement* ele = channelDoc.FirstChildElement()->FirstChildElement();
		ele != 0; ele = ele->NextSiblingElement())
	{
		std::string prop = ele->Value();
		if (prop.compare("program") == 0)
		{
			int passNum = 0;
			std::string vsPath;
			std::string fsPath;

			for (XMLElement* spHeader = ele->FirstChildElement(); 
				spHeader != 0; spHeader = spHeader->NextSiblingElement())
			{
				// vert and frag
				std::string spHeaderStr = spHeader->Value();
				if (spHeaderStr.compare("vertex") == 0)
				{
					vsPath = ParseXmlAttribute(*spHeader, "file", "");
				}
				else if (spHeaderStr.compare("fragment") == 0)
				{
					ShaderProgram* shaderProgram = new ShaderProgram();
					ShaderProgramInfo* shaderProgramInfo = new ShaderProgramInfo();

					fsPath = ParseXmlAttribute(*spHeader, "file", "");
					vsPath = "Data/" + vsPath;
					fsPath = "Data/" + fsPath;

					bool programLinked = shaderProgram->LoadFromFiles(vsPath.c_str(), fsPath.c_str(), "");

					if (!programLinked)
					{
						TODO("Load the invalid shader program");
						res->m_shaderPasses[passNum] = nullptr;
					}
					else
					{
						sShaderPass* pass = new sShaderPass();
						pass->m_programInfo = shaderProgramInfo;
						pass->m_program = shaderProgram;

						Renderer* r = Renderer::GetInstance();
						GLuint handle = shaderProgram->GetHandle();
						r->FillInfo(shaderProgramInfo, handle);

						res->m_shaderPasses[passNum] = pass;
					}

					vsPath = "";
					fsPath = "";
					passNum++;
				}
			}
		}
		else if (prop.compare("order") == 0)
		{
			int layerNum = ParseXmlAttribute(*ele, "layer", 0);		

			std::string queueStr = ParseXmlAttribute(*ele, "queue", "opague");
			eRenderQueue queue = ToRenderQueue(queueStr);

			res->m_layer = layerNum;
			res->m_queue = queue;
		}
		else if (prop.compare("blend") == 0)
		{
			for (XMLElement* header = ele->FirstChildElement(); 
				header != 0; header = header->NextSiblingElement())
			{
				std::string opAttr = ParseXmlAttribute(*header, "op", "");
				std::string srcAttr = ParseXmlAttribute(*header, "src", "");
				std::string destAttr = ParseXmlAttribute(*header, "dest", "");

				std::string blendProp = header->Value();

				// color
				if (blendProp.compare("color") == 0)
				{
					// operation
					if (opAttr == "add")
						res->m_state.m_colorBlendOp = COMPARE_ADD;

					// source
					if (srcAttr == "blend_one")
						res->m_state.m_colorSrcFactor = BLEND_ONE;
					else if (srcAttr == "blend_src_alpha")
						res->m_state.m_colorSrcFactor = BLEND_SRC_ALPHA;

					// destination
					if (destAttr == "blend_zero")
						res->m_state.m_colorDstFactor = BLEND_ZERO;
					else if (destAttr == "blend_one_minus_src_alpha")
						res->m_state.m_colorDstFactor = BLEND_ONE_MINUS_SRC_ALPHA;
				}
				// alpha
				else if (blendProp.compare("alpha") == 0)
				{
					// operation
					if (opAttr == "add")
						res->m_state.m_alphaBlendOp = COMPARE_ADD;

					// source
					if (srcAttr == "blend_src_alpha")
						res->m_state.m_alphaSrcFactor = BLEND_SRC_ALPHA;
					else if (srcAttr == "blend_one")
						res->m_state.m_alphaSrcFactor = BLEND_ONE;

					// destination
					if (destAttr == "blend_one_minus_src_alpha")
						res->m_state.m_alphaDstFactor = BLEND_ONE_MINUS_SRC_ALPHA;
					else if (destAttr == "blend_one")
						res->m_state.m_alphaDstFactor = BLEND_ONE;
				}
			}
		}
		else if (prop.compare("depth") == 0)
		{
			std::string compareStr = ParseXmlAttribute(*ele, "compare", "");

			size_t pos = compareStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueDepthCompareStr = compareStr.substr(0, pos);
				eDepthCompare trueDepthCompare = SelectCompare(trueDepthCompareStr);
				if (trueDepthCompare != NUM_COMPARE_MODE)
				{
					res->m_state.m_depthCompares.push_back(trueDepthCompare);
				}

				compareStr = compareStr.substr(pos + 1);
				pos = compareStr.find_first_of("|");
			}
			eDepthCompare compare = SelectCompare(compareStr);
			if (compare != NUM_COMPARE_MODE)
			{
				res->m_state.m_depthCompares.push_back(compare);
			}

			bool write = ParseXmlAttribute(*ele, "write", true);
			res->m_state.m_depthWrite = write;
		}
		else if (prop.compare("fill") == 0)
		{
			std::string modeStr = ParseXmlAttribute(*ele, "mode", "");
			if (modeStr.compare("wireframe") == 0)
			{
				res->m_state.m_fillMode = FILLMODE_WIRE;
			}
		}
		else if (prop.compare("wind") == 0)
		{
			std::string orderStr = ParseXmlAttribute(*ele, "order", "");

			size_t pos = orderStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueOrderStr = orderStr.substr(0, pos);
				eWindOrder trueOrder = SelectOrder(trueOrderStr);
				if (trueOrder != NUM_WIND_ORDER)
				{
					res->m_state.m_windOrders.push_back(trueOrder);
				}

				orderStr = orderStr.substr(pos + 1);
				pos = orderStr.find_first_of("|");
			}
			eWindOrder order = SelectOrder(orderStr);
			if (order != NUM_WIND_ORDER)
			{
				res->m_state.m_windOrders.push_back(order);
			}
		}
		else if (prop.compare("cull") == 0)
		{
			std::string faceStr = ParseXmlAttribute(*ele, "face", "");

			size_t pos = faceStr.find_first_of("|");
			while (pos != std::string::npos)
			{
				std::string trueFaceStr = faceStr.substr(0, pos);
				eCullMode trueCullMode = SelectCull(trueFaceStr);
				if (trueCullMode != NUM_CULL_MODE)
				{
					res->m_state.m_cullModes.push_back(trueCullMode);
				}

				faceStr = faceStr.substr(pos + 1);
				pos = faceStr.find_first_of("|");
			}
			eCullMode cull = SelectCull(faceStr);
			if (cull != NUM_CULL_MODE)
			{
				res->m_state.m_cullModes.push_back(cull);
			}
		}
		// Defaults
		else if (prop.compare("texture") == 0)
		{
			int idx = ParseXmlAttribute(*ele, "bind", -1);
			std::string name = ParseXmlAttribute(*ele, "name", "");
			std::string src = ParseXmlAttribute(*ele, "value", "");

			sShaderTexture* st = new sShaderTexture(idx, name, src);

			for (int passIdx = 0; passIdx < MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = res->m_shaderPasses[passIdx];
				
				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					pInfo->m_defaultTextures.push_back(st);
				}
			}
		}
		else if (prop.compare("color") == 0)
		{
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			sShaderProperty* sp = new sShaderProperty(type, name, value);

			for (int passIdx = 0; passIdx < MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = res->m_shaderPasses[passIdx];

				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					pInfo->m_defaultResources.push_back(sp);
				}
			}
		}
		else if (prop.compare("float") == 0)
		{
			// save as color due to structure
			std::string type = prop;
			std::string name = ParseXmlAttribute(*ele, "bind", "");
			std::string value = ParseXmlAttribute(*ele, "value", "");

			sShaderProperty* sp = new sShaderProperty(type, name, value);

			for (int passIdx = 0; passIdx < MAX_SHADER_PASS; ++passIdx)
			{
				sShaderPass* pass = res->m_shaderPasses[passIdx];

				if (pass != nullptr)
				{
					ShaderProgramInfo* pInfo = pass->m_programInfo;
					pInfo->m_defaultResources.push_back(sp);
				}
			}
		}
	}

	return res;
}


eDepthCompare ShaderChannel::SelectCompare(std::string compareStr)
{
	eDepthCompare res = NUM_COMPARE_MODE;

	if (compareStr.compare("less") == 0)
	{
		res = COMPARE_LESS;
	}
	else if (compareStr.compare("always") == 0)
	{
		res = COMPARE_ALWAYS;
	}
	else if (compareStr.compare("lequal") == 0)
	{
		res = COMPARE_LEQUAL;
	}
	else if (compareStr.compare("never") == 0)
	{
		res = COMPARE_NEVER;
	}
	else if (compareStr.compare("equal") == 0)
	{
		res = COMPARE_EQUAL;
	}
	else if (compareStr.compare("gequal") == 0)
	{
		res = COMPARE_GEQUAL;
	}
	else if (compareStr.compare("nequal") == 0)
	{
		res = COMPARE_NOT_EQUAL;
	}
	else if (compareStr.compare("greater") == 0)
	{
		res = COMPARE_GREATER;
	}

	return res;
}

eCullMode ShaderChannel::SelectCull(std::string cullStr)
{
	eCullMode res = NUM_CULL_MODE;

	if (cullStr.compare("front") == 0)
	{
		res = CULLMODE_FRONT;
	}
	else if (cullStr.compare("back") == 0)
	{
		res = CULLMODE_BACK;
	}
	else if (cullStr.compare("none") == 0)
	{
		res = CULLMODE_NONE;
	}

	return res;
}

eWindOrder ShaderChannel::SelectOrder(std::string orderStr)
{
	eWindOrder res = NUM_WIND_ORDER;

	if (orderStr.compare("ccw") == 0)
	{
		res = WIND_COUNTER_CLOCKWISE;
	}
	else if (orderStr.compare("cw") == 0)
	{
		res = WIND_CLOCKWISE;
	}

	return res;
}

void ShaderChannel::FillPropertyBlocks()
{
	for each (sShaderPass* pass in m_shaderPasses)
	{
		if (pass != nullptr)
		{
			ShaderProgramInfo* pInfo = pass->m_programInfo;
			std::vector<sPropertyBlockInfo*>& bInfo = pInfo->m_blockInfos;

			for each (sPropertyBlockInfo* info in bInfo)
			{
				PropertyBlock* block = new PropertyBlock();

				block->m_blockInfo = info;

				char* data = new char[info->blockSize]();
				block->m_dataBlock = data;
				
				pass->m_blocks.emplace(info->blockName, block);
			}

			// set initial value with SetProperty
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

					pass->SetProperty(name.c_str(), colorV4);
				}
				else if (type == "float")
				{
					float val = (float)(atof(value.c_str()));

					pass->SetProperty(name.c_str(), val);
				}
			}
		}
	}
}


void ShaderChannel::FillTextures()
{
	for each (sShaderPass* pass in m_shaderPasses)
	{
		if (pass != nullptr)
		{
			std::vector<sShaderTexture*>& textures = pass->m_programInfo->m_defaultTextures;

			for each (sShaderTexture* tex in textures)
			{
				int idx = tex->m_bindIdx;

				std::string bindName = tex->m_bindName;
				std::string bindSrc = tex->m_bindSrc;
				std::string fullPath = "Data/Images/" + bindSrc;

				Renderer* r = Renderer::GetInstance();
				Texture* texture = r->CreateOrGetTexture(fullPath);
				Sampler* sampler = new Sampler();
				texture->SetSampler(sampler);

				pass->m_textures.emplace(idx, texture);
			}
		}
	}
}


void sShaderPass::SetProperty(const char* name, void const* data, size_t datasize)
{
	sPropertyInfo* pInfo = m_programInfo->FindInfo(name);

	sPropertyBlockInfo* bInfo = nullptr;
	if (pInfo != nullptr)
	{
		 bInfo = pInfo->GetOwningBlock();
	}

	if (bInfo != nullptr)
	{
		PropertyBlock* block = CreateOrGetBlock(bInfo);

		ASSERT_RECOVERABLE(pInfo->size == datasize, "Property size does not match!");

		// Copy to GPU
		block->UpdateCPU(pInfo->offset, data, datasize);
	}
}


PropertyBlock* sShaderPass::CreateOrGetBlock(sPropertyBlockInfo* info)
{
	PropertyBlock* block = m_blocks[info->blockName];

	if (block != nullptr)
	{
		return block;
	}
	else
	{
		block = new PropertyBlock();
		block->m_blockInfo = info;

		m_blocks.emplace(info->blockName, block);

		return block;
	}
}