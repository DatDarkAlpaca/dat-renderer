#include "pch.hpp"
#include "renderer.hpp"

#include "utils/texture_loader.hpp"
#include "utils/file.hpp"

static constexpr u8 DefaultTexture[] = {
    180, 050, 050, 255,         200, 200, 200, 255,
    200, 200, 200, 255,         180, 050, 050, 255,
};


static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    auto const sourceString = [source]() 
    {
        switch (source)
        {
            case GL_DEBUG_SOURCE_API: 
                return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: 
                return "WINDOW SYSTEM";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: 
                return "SHADER COMPILER";
            case GL_DEBUG_SOURCE_THIRD_PARTY: 
                return "THIRD PARTY";
            case GL_DEBUG_SOURCE_APPLICATION: 
                return "APPLICATION";
            case GL_DEBUG_SOURCE_OTHER: 
                return "OTHER";
        }

        return "UNKNOWN";
    }();

    auto const typeString = [type]() 
    {
        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR: 
                return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: 
                return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: 
                return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: 
                return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: 
                return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER: 
                return "MARKER";
            case GL_DEBUG_TYPE_OTHER: 
                return "OTHER";
        }

        return "UNKNOWN";
    }();

    auto const severityString = [severity]() 
    {
        switch (severity) 
        {
            case GL_DEBUG_SEVERITY_NOTIFICATION: 
                return "NOTIFICATION";
            case GL_DEBUG_SEVERITY_LOW: 
                return "LOW";
            case GL_DEBUG_SEVERITY_MEDIUM: 
                return "MEDIUM";
            case GL_DEBUG_SEVERITY_HIGH: 
                return "HIGH";
        }

        return "UNKNOWN";
    }();

    std::cout << sourceString << ", " << typeString << ", " << severityString << ", " << id << ": " << message << '\n';
} 

static u32 renderer_create_pipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath)
{
    // Vertex:
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);

    std::string vertexCode = load_file(vertexFilepath);
    const char* vertexCodeC = vertexCode.c_str();
    glShaderSource(vertexShader, 1, &vertexCodeC, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment:
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentCode = load_file(fragmentFilepath);
    const char* fragmentCodeC = fragmentCode.c_str();
    glShaderSource(fragmentShader, 1, &fragmentCodeC, NULL);
    glCompileShader(fragmentShader);
        
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
        
    // Link:
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) 
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


static void renderer_process_quads(renderer_data &renderer)
{
    if(!renderer.quadsDirty)
        return;

    for(u32 i = 0; i < renderer.quadData.size(); ++i)
    {
        quad_data quadData = renderer.quadData[i];
        quadData.textureID = std::clamp<u32>(quadData.textureID, 0, renderer.textureCount - 1);
    }

    glNamedBufferSubData(renderer.instanceVBO, 0, sizeof(quad_data) * renderer.quadData.size(), renderer.quadData.data());

    renderer_draw_command drawCommand = {};
    drawCommand.count = 6;
    drawCommand.instanceCount = static_cast<u32>(renderer.quadData.size()) - 1;
    glNamedBufferSubData(renderer.DBO, 0, sizeof(renderer_draw_command), &drawCommand);

    renderer.quadsDirty = false;
}


void renderer_initialize(renderer_data& renderer, const renderer_arguments& arguments)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, nullptr);

    glCreateVertexArrays(1, &renderer.VAO);

    renderer.arguments = arguments;
    renderer.pipeline = renderer_create_pipeline(arguments.vertexFilepath, arguments.fragmentFilepath);
    renderer.quadData.reserve(arguments.maxInstanceCount);

    // Texture array:
    {
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &renderer.textureArray);

        u32 texCount = std::max<u32>(1, arguments.textureCount);
        glTextureStorage3D(renderer.textureArray, 1, GL_RGBA8, arguments.textureWidth, arguments.textureHeight, texCount);
    
        glTextureParameteri(renderer.textureArray, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(renderer.textureArray, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(renderer.textureArray, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(renderer.textureArray, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Default texture:
        renderer_add_texture(renderer, DefaultTexture);
    }

    // Draw Buffer:
    {
        glCreateBuffers(1, &renderer.DBO);

        renderer_draw_command drawCommand = {};
        drawCommand.count = 6;
        drawCommand.instanceCount = 0;

        glNamedBufferStorage(renderer.DBO, sizeof(renderer_draw_command), &drawCommand, GL_DYNAMIC_STORAGE_BIT);
    }

    // Per-Instance Data:
    {
        glCreateBuffers(1, &renderer.instanceVBO);
        const auto& iCount = renderer.arguments.maxInstanceCount;
        glNamedBufferStorage(renderer.instanceVBO, sizeof(quad_data) * iCount, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    // UBO:
    {
        glCreateBuffers(1, &renderer.viewProjectionUBO);
        glNamedBufferStorage(renderer.viewProjectionUBO, sizeof(glm::mat4) * 2, nullptr, GL_DYNAMIC_STORAGE_BIT);
    }

    // VBO:
    {
        glCreateBuffers(1, &renderer.VBO);

        float data[] = {
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f,
             0.5f, -0.5f, 0.0f,     1.0f, 0.0f,
             0.5f,  0.5f, 0.0f,     1.0f, 1.0f,
            -0.5f,  0.5f, 0.0f,     0.0f, 1.0f,
        };
    
        glNamedBufferStorage(renderer.VBO, sizeof(data), data, GL_DYNAMIC_STORAGE_BIT);
    }
    
    // EBO:
    {
        glCreateBuffers(1, &renderer.EBO);

        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
        glNamedBufferStorage(renderer.EBO, sizeof(indices), indices, GL_DYNAMIC_STORAGE_BIT);
    }
    
    // VAO Setup:
    glVertexArrayVertexBuffer(renderer.VAO, 0, renderer.VBO, 0, sizeof(vertex));
    glVertexArrayVertexBuffer(renderer.VAO, 1, renderer.instanceVBO, 0, sizeof(quad_data));
    glVertexArrayElementBuffer(renderer.VAO, renderer.EBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, renderer.viewProjectionUBO);

    {
        // Vertex positions
        glEnableVertexArrayAttrib(renderer.VAO, 0);
        glVertexArrayAttribFormat(renderer.VAO, 0, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));
        glVertexArrayAttribBinding(renderer.VAO, 0, 0);
        glVertexArrayBindingDivisor(renderer.VAO, 0, 0);

        // UVs
        glEnableVertexArrayAttrib(renderer.VAO, 1);
        glVertexArrayAttribFormat(renderer.VAO, 1, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, uvs));
        glVertexArrayAttribBinding(renderer.VAO, 1, 0);
        glVertexArrayBindingDivisor(renderer.VAO, 1, 0);

        // Per-instance transform
        for(u32 i = 0; i < 4; ++i)
        {
            glEnableVertexArrayAttrib(renderer.VAO, 2 + i);
            glVertexArrayAttribFormat(renderer.VAO, 2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * i);
            glVertexArrayAttribBinding(renderer.VAO, 2 + i, 1);
            glVertexArrayBindingDivisor(renderer.VAO, 1, 1);
        }

        // Per-instance texture index:
        glEnableVertexArrayAttrib(renderer.VAO, 6);
        glVertexArrayAttribFormat(renderer.VAO, 6, 1, GL_INT, GL_FALSE, offsetof(quad_data, textureID));
        glVertexArrayAttribBinding(renderer.VAO, 6, 1);
        glVertexArrayBindingDivisor(renderer.VAO, 6, 1);
    }
}

void renderer_begin(renderer_data &renderer)
{
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer_process_quads(renderer);

    glUseProgram(renderer.pipeline);
    glUniform1i(glGetUniformLocation(renderer.pipeline, "u_texture_array"), 0);

    glBindTextureUnit(0, renderer.textureArray);
    glBindVertexArray(renderer.VAO);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderer.DBO);
}

void renderer_draw(renderer_data &renderer)
{
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
}

void renderer_add_quad(renderer_data &renderer, const quad_data& quadData)
{
    renderer.quadsDirty = true;
    renderer.quadData.push_back(quadData);
}

void renderer_add_texture(renderer_data &renderer, const void *data)
{
    glTextureSubImage3D(
        renderer.textureArray, 0, 0, 0, renderer.textureCount, 
        renderer.arguments.textureWidth, renderer.arguments.textureHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE, data
    );

    ++renderer.textureCount;
}

void renderer_set_camera(renderer_data &renderer, const glm::mat4 &view, const glm::mat4 &projection)
{
    renderer.view = view;
    renderer.projection = projection;

    glm::mat4 data[2] = {
        view,
        projection 
    };
    
    glNamedBufferSubData(renderer.viewProjectionUBO, 0, sizeof(data), data);
}

void renderer_end(renderer_data &renderer)
{
    glUseProgram(0);
    glBindVertexArray(0);
}