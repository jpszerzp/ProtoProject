#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
const std::string Stringv(const char* format, va_list argList);

int nth_match_str(std::string subject, std::string object, int nth);	// find index of nth occurrence of object in subject;
																		// if no occurrence, return -1; if occurrence is smaller than nth, return INT_MA
																		// NOTE: string subject will be modified and is unsafe to refer after calling this function

bool StringIsNullOrEmpty(const char* str);
bool FindSubStringInString(const char* targetStr, const char* subjectStr);
bool IsStringInComment(const char* targetStr, const char* subjectStr);
bool IsStringInString(const char* targetStr, const char* subjectStr);
int  StringGetSize(const char* charArr);
bool StringSplitTwo(std::string splitted, const char* delimiter, std::string& prev, std::string& latter);