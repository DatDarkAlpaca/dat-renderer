#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>

inline std::string load_file(const std::filesystem::path& filepath)
{
    std::ifstream file(filepath);
    if(!file.is_open())
        throw;

    std::stringstream ss;
    ss << file.rdbuf();

    return ss.str();
}