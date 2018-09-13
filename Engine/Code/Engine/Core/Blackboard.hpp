#pragma once

#include "TinyXml2/tinyxml2.h"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Profiler/ProfilerNode.hpp"
//#include "Engine/Core/Util/DataUtils.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntRange.hpp"

#include <map>
#include <stack>

#define MAX_PROFILER_TREE_NODE 100
using namespace tinyxml2;

class Blackboard 
{
private:
	std::map<std::string, std::string>		 m_keyValuePairs;

public:
	std::map<ProfilerNode*, UInt64Vector2>		m_functionHpcInfoPairs;
	//std::stack<std::string>					m_profilerCallStack;
	float										m_lastFrameTime = -INFINITY;
	float										m_largestTime = -INFINITY;

	ProfilerNode*								m_profiledFunctionTree[MAX_PROFILER_TREE_NODE];
	//ProfilerHistory							m_history;
	std::deque<ProfilerHistory>					m_history;
	int											m_profiledFunctionIdx = 0;
	int											m_scopeIdx = 0;

public:
	Blackboard();
	~Blackboard();

	void			PopulateFromXmlElementAttributes( const XMLElement& element );
	void			SetValue( const std::string& keyName, const std::string& newValue );
	bool			GetValue( const std::string& keyName, bool defaultValue ) const;
	int				GetValue( const std::string& keyName, int defaultValue ) const;
	float			GetValue( const std::string& keyName, float defaultValue ) const;
	std::string		GetValue( const std::string& keyName, std::string defaultValue ) const;
	std::string		GetValue( const std::string& keyName, const char* defaultValue ) const;
	Rgba			GetValue( const std::string& keyName, const Rgba& defaultValue ) const;
	Vector2			GetValue( const std::string& keyName, const Vector2& defaultValue ) const;
	IntVector2		GetValue( const std::string& keyName, const IntVector2& defaultValue ) const;
	FloatRange		GetValue( const std::string& keyName, const FloatRange& defaultValue ) const;
	IntRange		GetValue( const std::string& keyName, const IntRange& defaultValue ) const;

	void MarkEndFrame();
	void ClearMap();
	void ClearIndex();
	void ClearTreeArray();
};