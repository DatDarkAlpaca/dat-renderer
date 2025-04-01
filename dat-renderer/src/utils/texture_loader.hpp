#pragma once
#include <filesystem>
#include <fstream>
#include <stb_image.h>

#include "common.hpp"

struct texture
{
    u8* data = nullptr;
    int width = 0, height = 0, channels = 0;
};

inline texture load_texture(const std::filesystem::path& filepath)
{
    int width, height, channels;
    u8* image = stbi_load(filepath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if(!image)
        throw;

    return { image, width, height, channels };
}

inline void unload_texture(texture& texture)
{
    stbi_image_free(texture.data);
    texture.data = nullptr;
    texture.width = 0;
    texture.height = 0;
    texture.channels = 0;
}