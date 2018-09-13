#pragma once

#include <fstream>
#include <iostream>

class File
{
public:
	std::fstream m_fs;
	const char*  m_path;

public:
	bool  WriteFile(const char* content);
	bool  ReadOpenFile();
	bool  WriteOpenFile();
	void  ReadCloseFile();
	void  WriteCloseFile();
	void  PrintFilef(const char* format, ...);			// file write
	bool  Readline(std::string& str);

	bool WriteTest();

	File();
	File(const char* path);
	~File();
};