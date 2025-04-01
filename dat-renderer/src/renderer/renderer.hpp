#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

#include "transform.hpp"
#include "common.hpp"

struct instance_data
{
    glm::mat4 transform;
    int textureID = 0;
};

struct vertex
{
    glm::vec3 position;
    glm::vec2 uvs;
};

struct renderer_draw_command
{
    u32 count;
    u32 instanceCount;
    u32 firstIndex;
    u32 baseIndex;
    u32 baseInstance;
};

struct renderer_arguments
{
    std::string vertexFilepath, fragmentFilepath;

    u32 maxInstanceCount = 0;
    u32 textureCount = 0;
    u32 textureWidth = 0, textureHeight = 0;
};

struct renderer_data
{
    glm::mat4 view = glm::mat4(1.0f), projection = glm::mat4(1.0f);
    renderer_arguments arguments;
    
    u32 VAO = invalid_handle;
    u32 VBO = invalid_handle;
    u32 EBO = invalid_handle;
    u32 textureArray = invalid_handle;

    u32 DBO = invalid_handle;
    u32 instanceVBO = invalid_handle;

    u32 pipeline = invalid_handle;
    u32 viewProjectionUBO = invalid_handle;

    u32 instanceCount = 0;
};

void renderer_initialize(renderer_data& renderer, const renderer_arguments& arguments);

void renderer_begin(renderer_data& renderer);

void renderer_add_quad(renderer_data& renderer, const transform& transform, int textureID = 0);

void renderer_set_camera(renderer_data& renderer, const glm::mat4& view = glm::mat4(1.0f), const glm::mat4& projection = glm::mat4(1.0f));

void renderer_end(renderer_data& renderer);