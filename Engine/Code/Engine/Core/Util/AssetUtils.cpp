#include "Engine/Core/Util/AssetUtils.hpp"
#include <windows.h>

#define MAX_PATH 260

std::string GetRunWinPath()
{
	TCHAR path[MAX_PATH + 1] = L"";
	GetCurrentDirectory(MAX_PATH, path);
	std::wstring pathWStr(&path[0]);
	std::string pathStr(pathWStr.begin(), pathWStr.end());
	static std::string subStr = "Code\\Game";
	std::size_t pos = pathStr.find(subStr);
	std::string runWinStr = pathStr.substr(0, pos);
	static std::string interStr = "Run_Win32\\";
	runWinStr.append(interStr);
	return runWinStr;
}

std::string GetAbsConfigPath()
{
	std::string path = GetRunWinPath();
	path.append("Data\\GameConfig.xml");
	return path;
}

static std::string GetSquirrelFixedFontPath()
{
	return "SquirrelFixedFont.png";
}

static std::string GetRelFontPath()
{
	return "Data\\Fonts\\";
}

std::string GetAbsFontPath()
{
	std::string path = GetRunWinPath();
	path.append(GetRelFontPath().append(GetSquirrelFixedFontPath()));
	return path;
}

std::string GetAbsFontPath(const std::string& fn)
{
	std::string path = GetRunWinPath();
	path.append(GetRelFontPath().append(fn));
	path.append(".png");
	return path;
}

static std::string GetRelImgPath()
{
	return "Data\\Images\\";
}

std::string GetAbsImgPath(const std::string& fn)
{
	std::string path = GetRunWinPath();
	path.append(GetRelImgPath().append(fn));
	path.append(".png");
	return path;
}

static std::string GetRelLogPath()
{
	return "Data\\Logs\\";
}

std::string GetAbsLogPath(const std::string& fn, const std::string& fmt)
{
	std::string path = GetRunWinPath();
	path.append(GetRelLogPath().append(fn));
	if (!fmt.empty())
		path.append(fmt);
	return path;
}

static std::string GetRelModelPath()
{
	return "Data\\Models\\";
}

std::string GetAbsModelPath(const std::string& fn)
{
	std::string path = GetRunWinPath();
	path.append(GetRelModelPath().append(fn));
	path.append(".obj");
	return path;
}

static std::string GetRelMatPath()
{
	return "Data\\Materials\\";
}

std::string GetAbsMatPath(const std::string& fn)
{
	std::string path = GetRunWinPath();
	path.append(GetRelMatPath().append(fn));
	path.append(".mat");
	return path;
}
