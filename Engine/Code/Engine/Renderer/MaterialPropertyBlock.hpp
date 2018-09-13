#pragma once

//#include "Engine/Renderer/UniformBuffer.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/MaterialProperty.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <vector>

struct sPropertyBlockInfo
{
	size_t maxsize; 

	std::vector<sPropertyInfo*> properties;
	std::string blockName;
	uint blockIdx;
	uint blockSize;

	void SetCount(uint propNum);

	sPropertyBlockInfo();
	~sPropertyBlockInfo();
};

class Renderer;

class PropertyBlock : public UniformBuffer
{
public:
	sPropertyBlockInfo* m_blockInfo;
	void* m_dataBlock;

public:
	void UpdateCPU(size_t offset, void const* data, size_t size);

	PropertyBlock();
	~PropertyBlock();
};