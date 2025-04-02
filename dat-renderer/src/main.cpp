#include "pch.hpp"
#include "common.hpp"

#include "renderer/renderer.hpp"

struct application_data
{
    renderer_data renderer;
    GLFWwindow* window = nullptr;
    u32 windowWidth = 0, windowHeight = 0;
};

static void initialize_window(application_data& data)
{
    if(!glfwInit())
        throw;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    data.window = glfwCreateWindow(data.windowWidth, data.windowHeight, "Dat Renderer", nullptr, nullptr);
    if (!data.window)
        throw;

    glfwMakeContextCurrent(data.window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw;
}

static void initialize_renderer(application_data& data, renderer_arguments arguments) 
{
    renderer_initialize(data.renderer, arguments);

    // Second texture:
    static constexpr u8 secondTexture[] = {
        060, 050, 050, 255,          90, 200,  90, 255,
         90, 200,  90, 255,          60, 050, 050, 255,
    };
    renderer_add_texture(data.renderer, secondTexture);

    // Camera:
    auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projection = glm::ortho(0.0f, (float)data.windowWidth, (float)data.windowHeight, 0.0f);
    renderer_set_camera(data.renderer, view, projection);
}

static void initialize_scene(application_data& data)
{  
    constexpr u32 quadAmountX = 120;
    constexpr u32 quadAmountY = 120;

    float width  = (float)data.windowWidth / (float)quadAmountX;
    float height = width;

    for(u32 x = 0; x < quadAmountX; ++x)
    {
        for(u32 y = 0; y < quadAmountY; ++y)
        {
            transform transform = {};
            transform.position = glm::vec3(width / 2.0f + x * width, height / 2.0f + y * height, 0.0f);
            transform.scale = glm::vec3(width, height, 1.f);
    
            quad_data quadData = {};
            quadData.transform = transform.get_transform();
            quadData.textureID = std::rand() % (1 + 1 - 0) + 0;
            renderer_add_quad(data.renderer, quadData);
        }
    }
}

static void update(application_data& data)
{
    renderer_begin(data.renderer);

    renderer_draw(data.renderer);

    renderer_end(data.renderer);
}

int main()
{
    std::srand(static_cast<u32>(std::time(nullptr)));

    application_data appData;
    {
        appData.windowWidth = 1280;
        appData.windowHeight = 920;
    }

    initialize_window(appData);

    renderer_arguments arguments;
    {
        arguments.vertexFilepath    = "res/quad.vert";
        arguments.fragmentFilepath  = "res/quad.frag";
        arguments.maxInstanceCount  = 120 * 120;
        arguments.textureCount      = 2;
        arguments.textureWidth      = 2;
        arguments.textureHeight     = 2;
    }
    initialize_renderer(appData, arguments);

    initialize_scene(appData);

    while(!glfwWindowShouldClose(appData.window))
    {
        glfwPollEvents();

        update(appData);

        glfwSwapBuffers(appData.window);
    }

    return 0;
}