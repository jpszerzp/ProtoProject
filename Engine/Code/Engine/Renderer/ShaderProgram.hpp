#pragma once

#include "Engine/Renderer/GLFunctions.hpp"

class ShaderProgram
{
private:
	GLuint m_programHandle;

public:
	bool LoadFromFiles( char const *root, const char* delimited );
	bool LoadFromFiles(const char* vsPath, const char* fsPath, const char* delimited);
	bool LoadFromStrings( char const* vs, char const* fs, const char* delimited = "");
	void Reload(const char* shaderName, const char* delimited);

	// getters
	GLuint GetHandle() { return m_programHandle; }
};

void* FileReadToNewBuffer(char const* path);