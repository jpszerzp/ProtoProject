#include "Engine/Core/Util/StringUtils.hpp"
#include <stdarg.h>
#include <limits.h>

//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

const std::string Stringv(const char* format, va_list argList)
{
	char textLiteral[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, argList );	
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


int nth_match_str(std::string subject, std::string object, int nth)
{
	int posCount = 0;
	size_t globalOccurence = 0;

	while(subject.find(object) != std::string::npos)
	{
		size_t occurence = subject.find(object);
		size_t startIndex = occurence + object.length();
		globalOccurence += startIndex;
		posCount++;
		subject = subject.substr(startIndex);

		if (posCount == nth)
		{
			return (int)(globalOccurence - 1);
		}
	}

	if (posCount == 0)
	{
		return -1;
	}
	else 
	{
		return INT_MAX;
	}
}


bool StringIsNullOrEmpty(const char* str)
{
	std::string id(str);
	return id.empty();
}


bool FindSubStringInString(const char*, const char*)
{
	return false;
}


bool IsStringInComment(const char*, const char*)
{
	return false;

}


bool IsStringInString(const char*, const char*)
{
	return false;

}

int StringGetSize(const char* charArr)
{
	std::string str(charArr);
	return (int)str.size();
}

void StringSplitTwo(std::string splitted, const char* delimiter, std::string& prev, std::string& latter)
{
	std::string delimiterStr(delimiter);
	size_t delimiterLength = delimiterStr.length();

	size_t delimiterIdx = splitted.find(delimiter);		// index of delimiter
	size_t stringSize = splitted.length();					// size of splitted string
	int prevLength = (int)delimiterIdx;						// size of string before delimiter

	// fill prev and latter holder
	prev = splitted.substr(0, delimiterIdx);
	latter = splitted.substr(delimiterIdx + delimiterLength, stringSize - (delimiterLength + prevLength));
}
