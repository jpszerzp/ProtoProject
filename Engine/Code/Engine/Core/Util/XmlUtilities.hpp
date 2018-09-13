#pragma once

#include "TinyXml2/tinyxml2.h"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/IntVector2.hpp"

#include <string>

using namespace tinyxml2; 

int ParseXmlAttribute( const XMLElement& element, const char* attributeName, int defaultValue );
char ParseXmlAttribute( const XMLElement& element, const char* attributeName, char defaultValue );
bool ParseXmlAttribute( const XMLElement& element, const char* attributeName, bool defaultValue );
float ParseXmlAttribute( const XMLElement& element, const char* attributeName, float defaultValue );
Rgba ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Rgba& defaultValue );
Vector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const Vector2& defaultValue );
IntRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntRange& defaultValue );
FloatRange ParseXmlAttribute( const XMLElement& element, const char* attributeName, const FloatRange& defaultValue );
IntVector2 ParseXmlAttribute( const XMLElement& element, const char* attributeName, const IntVector2& defaultValue );
std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const std::string& defaultValue );
std::string ParseXmlAttribute( const XMLElement& element, const char* attributeName, const char* defaultValue = nullptr );