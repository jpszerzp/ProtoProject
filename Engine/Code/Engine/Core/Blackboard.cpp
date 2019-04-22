#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/Time/TheTime.hpp"

#include <cstring>

Blackboard::Blackboard()
{
	
}

Blackboard::~Blackboard()
{

}

void Blackboard::PopulateFromXmlElementAttributes( const XMLElement& element )
{
	for(const XMLAttribute* attr = element.FirstAttribute(); attr != 0; attr = attr->Next())
	{
		m_keyValuePairs.emplace(attr->Name(), attr->Value());
	}
}

IntRange Blackboard::GetValue( const std::string& keyName, const IntRange& defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	IntRange res = defaultValue;
	res.SetFromText(val);
	return res;
}

FloatRange Blackboard::GetValue( const std::string& keyName, const FloatRange& defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	FloatRange res = defaultValue;
	res.SetFromText(val);
	return res;
}

IntVector2 Blackboard::GetValue( const std::string& keyName, const IntVector2& defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	IntVector2 res = defaultValue;
	res.SetFromText(val);
	return res;
}

Vector2 Blackboard::GetValue( const std::string& keyName, const Vector2& defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	Vector2 res = defaultValue;
	res.SetFromText(val);
	return res;
}

Rgba Blackboard::GetValue( const std::string& keyName, const Rgba& defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	Rgba res = defaultValue;
	res.SetFromText(val);
	return res;
}

std::string Blackboard::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	if (val == nullptr)
	{
		return defaultValue;
	}
	else
	{
		return val;
	}
}

std::string	Blackboard::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	std::string val = m_keyValuePairs.find(keyName)->second;
	if (val == "")
	{
		return defaultValue;
	}
	else 
	{
		return val;
	}
}

float Blackboard::GetValue( const std::string& keyName, float defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	float res = defaultValue;
	res = static_cast<float>(atof(val));
	return res;
}

int Blackboard::GetValue( const std::string& keyName, int defaultValue ) const
{
	const char* val = (m_keyValuePairs.find(keyName)->second).c_str();
	int res = defaultValue;
	res = atoi(val);
	return res;
}

bool Blackboard::GetValue( const std::string& keyName, bool defaultValue ) const
{
	std::string val = m_keyValuePairs.find(keyName)->second;
	bool res = defaultValue;
	if (val == "true")
	{
		res = true;
	}
	else if (val == "false")
	{
		res = false;
	}
	return res; 
}

/*
void Blackboard::MarkEndFrame()
{
	//// end of "last" frame
	//for (std::map<ProfilerNode*, UInt64Vector2>::iterator it = m_functionHpcInfoPairs.begin();
	//	it != m_functionHpcInfoPairs.end(); ++it)
	//{
	//	UInt64Vector2 startEnd;
	//	if (it->first->m_tag == "TheApp::RunFrame")
	//	{
	//		startEnd = it->second;
	//		uint64_t start = startEnd.ux;
	//		uint64_t end = startEnd.uy;
	//		uint64_t diff = end - start;
	//		float ms = (float)PerformanceCountToSeconds(diff) * 1000.f;
	//		m_lastFrameTime = ms;

	//		break;
	//	}
	//}

	//// clear history
	//for each (ProfilerHistory history in m_history)
	//{
	//	history.Clear();
	//}

	//ProfilerHistory history = ProfilerHistory(m_functionHpcInfoPairs);

	//// by default we just keep last frame, history number is 1
	//m_history.push_back(history);
	//if (m_history.size() > 1U)
	//{
	//	ProfilerHistory poppedHistory = m_history.front();
	//	poppedHistory.Clear();

	//	m_history.pop_front();
	//}
}
*/

/*
void Blackboard::ClearMap()
{
	m_functionHpcInfoPairs.clear();
}

void Blackboard::ClearIndex()
{
	m_profiledFunctionIdx = 0;
	m_scopeIdx = 0;
}

void Blackboard::ClearTreeArray()
{
	for (int idx = 0; idx < MAX_PROFILER_TREE_NODE; ++idx)
	{
		ProfilerNode* node = m_profiledFunctionTree[idx];
		if (node != nullptr)
		{
			//delete node;
			node = nullptr;
		}
		else
		{
			break;
		}
	}
}
*/

void Blackboard::SetValue( const std::string& keyName, const std::string& newValue )
{
	std::map<std::string, std::string>::iterator it = m_keyValuePairs.find(keyName); 
	if (it != m_keyValuePairs.end())
		it->second = newValue;
}
