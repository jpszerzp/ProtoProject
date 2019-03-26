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


void Blackboard::SetValue( const std::string& keyName, const std::string& newValue )
{
	std::map<std::string, std::string>::iterator it = m_keyValuePairs.find(keyName); 
	if (it != m_keyValuePairs.end())
		it->second = newValue;
}
