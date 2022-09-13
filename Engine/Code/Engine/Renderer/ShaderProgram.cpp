#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"

#include <string>
#include <fstream>
#include <stdio.h>
#include <vector>

static std::string ExpandInclude(std::vector<std::string>& includeLst, std::string src)
{
	std::string includeRef = "#include";
	size_t firstIncludeIdx = src.find(includeRef);

	if (firstIncludeIdx != std::string::npos)
	{
		std::string textBelowAndWithInclude = src.substr(firstIncludeIdx, src.length() - firstIncludeIdx);

		std::string textAboveInclude = src.substr(0, firstIncludeIdx - 0);

		bool inComment = false;

		// see if include is in comment // 
		std::string commentKey_0 = "//";
		std::size_t keyFound_0 = textAboveInclude.rfind(commentKey_0);
		if (keyFound_0 != std::string::npos)
		{
			std::string inBetweenIncludeAndSlash = textAboveInclude.substr(keyFound_0, textAboveInclude.length() - keyFound_0);

			// if there is no new line, then include IS in comment
			if (inBetweenIncludeAndSlash.find('\n') == std::string::npos)
			{
				inComment = true;
			}
		}

		// see if include is in comment /*
		std::string commentKey_1 = "/*";
		std::size_t keyFound_1 = textAboveInclude.rfind(commentKey_1);
		if (keyFound_1 != std::string::npos)
		{
			// for /*, no need to care for new line
			inComment = true;
		}

		if (!inComment)
		{
			// not in comment

			std::string includeLine(textBelowAndWithInclude.begin(), std::find(textBelowAndWithInclude.begin(), textBelowAndWithInclude.end(), '\n'));
			std::string textBelowInclude(std::find(textBelowAndWithInclude.begin(), textBelowAndWithInclude.end(), '\n'), textBelowAndWithInclude.end());

			// file is not included yet
			if (std::find(includeLst.begin(), includeLst.end(), includeLine) == includeLst.end())
			{
				// "#include "inc/A.glsl""
				includeLst.push_back(includeLine);

				// "inc/A.glsl"
				std::string bareFileSubpath(std::find(includeLine.begin(), includeLine.end(), '"') + 1, includeLine.end() - 1);

				std::string fullPath = GetRunWinPath();
				std::string interPath = "Data\\Shaders\\glsl\\" + bareFileSubpath;
				fullPath.append(interPath);
				char* includedSrc = (char*)FileReadToNewBuffer(fullPath.c_str());

				std::string includedSrcStr(includedSrc);
				std::string expandedSrc = ExpandInclude(includeLst, includedSrcStr);

				//std::string allText;
				std::string firstIncludeText;
				if (expandedSrc == "")
				{
					std::string includedStr(includedSrc);
					firstIncludeText = textAboveInclude + includedStr;
				}
				else
				{
					firstIncludeText = textAboveInclude + expandedSrc;
				}

				// finished expand the first include, expand remained includes in the same way
				std::string remainedIncludeText = ExpandInclude(includeLst, textBelowInclude);

				std::string allText = firstIncludeText + remainedIncludeText;

				return allText;
			}
			else
			{
				// file already included
				// ignore that include line
				// and continue expand、
				std::string remainedIncludeText = ExpandInclude(includeLst, textBelowInclude);

				std::string allText = textAboveInclude + remainedIncludeText;

				return allText;
			}
		}
		else
		{
			// in comment

			std::string includeLine(textBelowAndWithInclude.begin(), std::find(textBelowAndWithInclude.begin(), textBelowAndWithInclude.end(), '\n'));
			std::string textBelowInclude(std::find(textBelowAndWithInclude.begin(), textBelowAndWithInclude.end(), '\n'), textBelowAndWithInclude.end());

			std::string remainedIncludeText = ExpandInclude(includeLst, textBelowInclude);

			std::string allText = textAboveInclude + includeLine + remainedIncludeText;

			return allText;
		}
	}
	else
	{
		// no more includes

		return src;
	}
}


static std::string AddDelimitedToShaderSrc(const char* src, const char* delimited, int& out_errorLineOffset)
{
	std::string newSrc(src);
	std::string versionRef = "#version";
	size_t headerIndex = newSrc.find(versionRef);
	std::string versionHeader = newSrc.substr(headerIndex, newSrc.length() - headerIndex);
	
	// if we can find #version we continue pending
	if (versionHeader != "")
	{
		std::string firstLine(versionHeader.begin(), std::find(versionHeader.begin(), versionHeader.end(), '\n'));
		std::string remainLines(std::find(versionHeader.begin(), versionHeader.end(), '\n'), versionHeader.end());

		std::string res = "";
		if ((delimited != NULL) && (delimited[0] != '\0'))
		{
			res += "#define ";
			std::string delimitedStr(delimited);
			while (delimitedStr.find(";") != std::string::npos)
			{
				size_t length = delimitedStr.length();
				size_t pos = delimitedStr.find(";");
				std::string sub = delimitedStr.substr(0, pos);
				if (sub.find("=") != std::string::npos)
				{
					size_t equalIndex = sub.find("=");
					sub.replace(equalIndex, 1, " ");
				}
				res += sub;
				res += "\n#define ";
				delimitedStr = delimitedStr.substr(pos + 1, length);

				out_errorLineOffset++;
			}

			if (delimitedStr.find("=") != std::string::npos)
			{
				size_t equalIndex = delimitedStr.find("=");
				delimitedStr.replace(equalIndex, 1, " ");
			}
			res += delimitedStr;
			out_errorLineOffset++;
		}			
		// finishing result string from defines

		// See if the result string is empty
		// If yes, we simply ignore res
		// If not, pend the result after first line
		if (res != "")
		{
			newSrc = firstLine + "\n" + res + remainLines;
		}
		else 
		{
			newSrc = firstLine + remainLines;
		}
	}
	
	return newSrc;
}


static void LogShaderError(GLuint shader_id, char* fp, int errorLineOffset)
{
	// figure out how large the buffer needs to be
	GLint length;
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// make a buffer, and copy the log to it. 
	char *buffer = new char[length + 1];
	glGetShaderInfoLog(shader_id, length, &length, buffer);

	// Print it out (may want to do some additional formatting)
	buffer[length] = NULL;

	std::string bufferStr(buffer);
	bufferStr = bufferStr.substr(0, bufferStr.length() - 1);				// trim extra 1 new line
	std::string fpStr(fp);
	std::string theError = "";

	while(bufferStr.find('\n') != std::string::npos)
	{
		theError += fpStr;

		size_t newLineIndex = bufferStr.find('\n');
		std::string firstLine = bufferStr.substr(0, newLineIndex);
		bufferStr = bufferStr.substr(newLineIndex + 1, bufferStr.length() - newLineIndex);

		int firstMatchIndex = nth_match_str(firstLine, ":", 1);
		int secondMatchIndex = nth_match_str(firstLine, ":", 2);
		int thirdMatchIndex = nth_match_str(firstLine, ":", 3);
		std::string lineNumStr = firstLine.substr(secondMatchIndex + 1, (thirdMatchIndex - 1) - secondMatchIndex);
		int lineNum = stoi(lineNumStr) - errorLineOffset;
		std::string errMsg = firstLine.substr(0, firstMatchIndex + 1) + firstLine.substr(thirdMatchIndex + 1);

		theError += (" (" + std::to_string(lineNum) + "): ");		// line num
		theError += errMsg;											// error msg
		theError += "\n";											// new line
	}

	DebuggerPrintf( theError.c_str() );
	ASSERT_RECOVERABLE(0, theError.c_str());

	// free up the memory we used. 
	delete buffer;
}


static void LogProgramError(GLuint program_id)
{
	// get the buffer length
	GLint length;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);

	// copy the log into a new buffer
	char *buffer = new char[length + 1];
	glGetProgramInfoLog(program_id, length, &length, buffer);

	// print it to the output pane
	buffer[length] = NULL;
	DebuggerPrintf(buffer);
	ASSERT_RECOVERABLE(0, buffer);

	// cleanup
	delete buffer;
}


static GLuint LoadShaderFromString(char const* src, GLenum type, const char* delimited)
{
	std::string name = "built in";
	int errorLineOffset = 0;

	std::string newSrc = AddDelimitedToShaderSrc(src, delimited, errorLineOffset);

	// Create a shader
	GLuint shader_id = glCreateShader(type);
	ASSERT_OR_DIE(shader_id != NULL, "creation of shader fails!");

	char* newSrcCharArr = (char*)(newSrc.c_str());

	// Bind source to it, and compile
	// You can add multiple strings to a shader – they will 
	// be concatenated together to form the actual source object.
	GLint shader_length = (GLint)strlen(newSrcCharArr);
	glShaderSource(shader_id, 1, &newSrcCharArr, &shader_length);
	glCompileShader(shader_id);

	// Check status
	GLint status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		LogShaderError(shader_id, (char*)(name.c_str()), errorLineOffset); // function we write
		glDeleteShader(shader_id);
		shader_id = NULL;
	}

	return shader_id;
}


static GLuint LoadShader( char const *filename, GLenum type,
	const char* delimited, std::vector<std::string>& )
{
	int errorLineOffset = 0;

	char* src = (char*)FileReadToNewBuffer(filename);
	ASSERT_OR_DIE(src != nullptr, "shader file name is null!");

	// includes 
	std::vector<std::string> filesToInclude;
	std::string srcStr(src);
	std::string includeSrc = ExpandInclude(filesToInclude, srcStr);

	// defines
	std::string defineStr = AddDelimitedToShaderSrc(includeSrc.c_str(), delimited, errorLineOffset);

	// Create a shader
	GLuint shader_id = glCreateShader(type);
	ASSERT_OR_DIE(shader_id != NULL, "creation of shader fails!");

	char* newSrcCharArr = (char*)(defineStr.c_str());

	// Bind source to it, and compile
	// You can add multiple strings to a shader – they will 
	// be concatenated together to form the actual source object.
	//GLint shader_length = (GLint)strlen(src);
	GLint shader_length = (GLint)strlen(newSrcCharArr);
	glShaderSource(shader_id, 1, &newSrcCharArr, &shader_length);
	glCompileShader(shader_id);

	// Check status
	GLint status;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		LogShaderError(shader_id, (char*)filename, errorLineOffset); // function we write
		glDeleteShader(shader_id);
		shader_id = NULL;
	}

	free(src);

	return shader_id;
}


static GLuint CreateAndLinkProgram( GLint vs, GLint fs )
{
	// create the program handle - how you will reference
	// this program within OpenGL, like a texture handle
	GLuint program_id = glCreateProgram();
	ASSERT_OR_DIE( program_id != 0, "creation of shader program fails!" );

	// Attach the shaders you want to use
	glAttachShader( program_id, vs );
	glAttachShader( program_id, fs );

	// Link the program (create the GPU program)
	glLinkProgram( program_id );

	// Check for link errors - usually a result
	// of incompatibility between stages.
	GLint link_status;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);

	if (link_status == GL_FALSE) {
		LogProgramError(program_id);
		glDeleteProgram(program_id);
		program_id = 0;
	} 

	// no longer need the shaders, you can detach them if you want
	// (not necessary)
	glDetachShader( program_id, vs );
	glDetachShader( program_id, fs );

	return program_id;
}


bool ShaderProgram::LoadFromFiles( char const *root, const char* delimited )
{
	std::string vs_file = root;
	vs_file += ".vs"; 

	std::string fs_file = root; 
	fs_file += ".fs"; 

	std::vector<std::string> vertIncludeFilepaths;
	std::vector<std::string> fragIncludeFilepaths;

	// Compile the two stages we're using (all shaders will implement the vertex and fragment stages)
	// later on, we can add in more stages;
	GLuint vert_shader = LoadShader( vs_file.c_str(), GL_VERTEX_SHADER, delimited, vertIncludeFilepaths ); 

	GLuint frag_shader = LoadShader( fs_file.c_str(), GL_FRAGMENT_SHADER, delimited, fragIncludeFilepaths ); 

	// Link the program
	// program_handle is a member GLuint. 
	m_programHandle = CreateAndLinkProgram( vert_shader, frag_shader ); 
	glDeleteShader( vert_shader ); 
	glDeleteShader( frag_shader ); 

	return (m_programHandle != NULL); 
};


bool ShaderProgram::LoadFromFiles(const char* vsPath, const char* fsPath, const char* delimited)
{
	std::vector<std::string> vertIncludeFilepaths;
	std::vector<std::string> fragIncludeFilepaths;

	GLuint vert_shader = LoadShader( vsPath, GL_VERTEX_SHADER, delimited, vertIncludeFilepaths ); 
	GLuint frag_shader = LoadShader( fsPath, GL_FRAGMENT_SHADER, delimited, fragIncludeFilepaths ); 

	m_programHandle = CreateAndLinkProgram( vert_shader, frag_shader ); 
	glDeleteShader( vert_shader ); 
	glDeleteShader( frag_shader ); 

	return (m_programHandle != NULL);
}


bool ShaderProgram::LoadFromStrings(char const* vs, char const* fs, const char* delimited)
{
	GLuint vert_shader = LoadShaderFromString(vs, GL_VERTEX_SHADER, delimited);
	GLuint frag_shader = LoadShaderFromString(fs, GL_FRAGMENT_SHADER, delimited);
	m_programHandle = CreateAndLinkProgram(vert_shader, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	return (m_programHandle != NULL);
}


void* FileReadToNewBuffer(char const* path)
{
	FILE *fp = nullptr;
	fopen_s( &fp, path, "r" );

	if (fp == nullptr) 
	{
		return nullptr;
	}

	size_t size = 0U;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	unsigned char* buffer = (unsigned char*)malloc(size + 1U);

	size_t read = fread( buffer, 1, size, fp );
	fclose(fp);

	buffer[read] = NULL;

	return buffer; 
}


void ShaderProgram::Reload(const char* shaderName, const char* delimited)
{
	std::string sn(shaderName);
	std::string path = GetRunWinPath();
	std::string fp = "Data\\Shaders\\" + sn;
	path.append(fp);
	LoadFromFiles(path.c_str(), delimited);

	std::string printMsg = sn + " reloaded\n";
	DebuggerPrintf(printMsg.c_str());
}