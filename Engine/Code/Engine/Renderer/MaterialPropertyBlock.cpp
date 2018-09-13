#pragma once

#include "Engine/Renderer/MaterialPropertyBlock.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

sPropertyBlockInfo::sPropertyBlockInfo()
{

}


sPropertyBlockInfo::~sPropertyBlockInfo()
{
	//for each (sPropertyInfo* binfo in properties)
	//{
	//	delete binfo;
	//	binfo = nullptr;
	//}

	//properties.clear();
}


void sPropertyBlockInfo::SetCount(uint propNum)
{
	// Add an empty property info for each incoming property
	for (uint i = 0; i < propNum; ++i)
	{
		sPropertyInfo* propInfo = new sPropertyInfo();
		properties.push_back(propInfo);
	}
}


PropertyBlock::PropertyBlock()
{

}


PropertyBlock::~PropertyBlock()
{
	TODO("how to release these memory?");
	//delete m_blockInfo;
	//m_blockInfo = nullptr;

	//delete m_dataBlock;
	//m_dataBlock = nullptr;
}


void PropertyBlock::UpdateCPU(size_t offset, void const* data, size_t dataSize)
{
	char* ptr = (char*)(m_dataBlock);
	memcpy(ptr + offset, data, dataSize);
}
