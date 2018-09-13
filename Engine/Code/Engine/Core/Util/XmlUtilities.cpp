#include "Engine/Core/Util/XmlUtilities.hpp"

int ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue )
{
	int res = element.IntAttribute(attributeName, defaultValue);
	return res;
}

char ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue )
{
	const char* res = element.Attribute(attributeName);
	if (res == nullptr)
	{
		return defaultValue;
	}
	else 
	{
		return res[0];
	}
}

bool ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue )
{
	bool res = element.BoolAttribute(attributeName, defaultValue);
	return res;
}

float ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue )
{
	float res = element.FloatAttribute(attributeName, defaultValue);
	return res;
}

Rgba ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue )
{
	Rgba res = defaultValue;
	const char* attribute = element.Attribute(attributeName);
	if (attribute != nullptr)
	{
		res.SetFromText(attribute);
	}
	return res;
}

Vector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue )
{
	Vector2 res = defaultValue;
	const char* attribute = element.Attribute(attributeName);
	if (attribute != nullptr)
	{
		res.SetFromText(attribute);
	}
	return res;
}

IntRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue )
{
	IntRange res = defaultValue;
	const char* attribute = element.Attribute(attributeName);
	if (attribute != nullptr)
	{
		res.SetFromText(attribute);
	}
	return res;
}

FloatRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue )
{
	FloatRange res = defaultValue;
	const char* attribute = element.Attribute(attributeName);
	if (attribute != nullptr)
	{
		res.SetFromText(attribute);
	}
	return res;
}

IntVector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue )
{
	IntVector2 res = defaultValue;
	const char* attribute = element.Attribute(attributeName);
	if (attribute != nullptr)
	{
		res.SetFromText(attribute);
	}
	return res;
}

std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue )
{
	const char* res = element.Attribute(attributeName);
	if (res == nullptr)
	{
		return defaultValue;
	}
	else 
	{
		std::string strRes(res);
		return strRes;
	}
}

std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue)
{
	if (defaultValue != nullptr)
	{
		std::string str(defaultValue);
		return ParseXmlAttribute(element, attributeName, str);
	}
	else
	{
		return ParseXmlAttribute(element, attributeName, "");
	}
}