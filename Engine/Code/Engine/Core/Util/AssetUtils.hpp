#pragma once

#include <string>

std::string GetRunWinPath();
std::string GetAbsConfigPath();
std::string GetAbsFontPath();
std::string GetAbsFontPath(const std::string& fn);
std::string GetAbsImgPath(const std::string& fn);
std::string GetAbsLogPath(const std::string& fn, const std::string& fmt = "");
std::string GetAbsModelPath(const std::string& fn);
std::string GetAbsMatPath(const std::string& fn);