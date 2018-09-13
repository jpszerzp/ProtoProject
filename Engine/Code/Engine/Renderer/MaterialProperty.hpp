#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vector4.hpp"

#include <string>
#include <map>
#include <vector>

struct sPropertyBlockInfo;

struct sPropertyInfo
{
	sPropertyBlockInfo* m_owningBlock = nullptr;

	std::string name;
	size_t offset;
	size_t size;

	sPropertyInfo();
	~sPropertyInfo();

	sPropertyBlockInfo* GetOwningBlock() { return m_owningBlock; }
};

// This is separate from property block approach
class Property
{
public:
	virtual void Bind(uint handle) = 0;
	
public:
	std::string m_name;
};

class PropertyVec4 : public Property
{
public:
	PropertyVec4(const char* name, const Vector4& value);
	~PropertyVec4(){}
	
	virtual void Bind(uint handle) override;

public:
	Vector4 m_value;
};