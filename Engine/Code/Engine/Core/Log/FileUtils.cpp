#include "Engine/Core/Log/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <stdarg.h>

File::File(const char* path)
{
	m_path = path;
}

File::File()
{

}

File::~File()
{

}

// open file, write to it and close it
bool File::WriteFile(const char* content)
{
	WriteOpenFile();

	if(!m_fs)
	{
		std::cerr<<"Cannot open the output file."<<std::endl;
		return false;
	}

	m_fs.write(content, sizeof(content)/sizeof(content[0]));

	WriteCloseFile();
	return true;
}


bool File::ReadOpenFile()
{
	m_fs.open(m_path, std::ios::in);

	return m_fs.is_open();
}

bool File::WriteOpenFile()
{
	m_fs.open(m_path, std::ios::out | std::ios::app);

	return m_fs.is_open();
}

void File::ReadCloseFile()
{
	m_fs.close();
}

void File::WriteCloseFile()
{
	m_fs.close();
}

bool File::WriteTest()
{
	WriteOpenFile();

	if(!m_fs)
	{
		std::cerr<<"Cannot open the output file."<<std::endl;
		return false;
	}

	for (uint i = 0; i < 12000000; ++i)
	{
		uint randInt = GetRandomIntLessThan(1000000);
		std::string intStr = Stringf("%i", randInt);
		m_fs.write(intStr.c_str(), 1);
	}

	WriteCloseFile();
	return true;
}


void File::PrintFilef(const char* format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	std::string str = Stringv(format, variableArgumentList);
	m_fs.write(str.c_str(), str.size());
}

bool File::Readline(std::string& str)
{
	if (std::getline(m_fs, str))
	{
		return true;
	}
	
	return false;
}
