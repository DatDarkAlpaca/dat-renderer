#include "pch.hpp"
#include "common.hpp"

#include "renderer/renderer.hpp"

struct application_data
{
    GLFWwindow* window = nullptr;
    renderer_data renderer;
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

    data.window = glfwCreateWindow(600, 480, "Dat Renderer", nullptr, nullptr);
    if (!data.window)
        throw;

    glfwMakeContextCurrent(data.window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw;
}

static void initialize_renderer(application_data& data) {
    renderer_arguments arguments;
    {
        arguments.vertexFilepath    = "res/quad.vert";
        arguments.fragmentFilepath  = "res/quad.frag";
        arguments.maxInstanceCount  = 100;
        arguments.textureCount      = 2;
        arguments.textureWidth      = 2;
        arguments.textureHeight     = 2;
    }
    renderer_initialize(data.renderer, arguments);

    // Camera:
    auto view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    auto projection = glm::ortho(0.0f, 600.0f, 480.0f, 0.0f);
    renderer_set_camera(data.renderer, view, projection);

    // Quad 0:
    transform transform = {};
    transform.position = glm::vec3(50.f, 50.f, 1.0f);
    transform.scale = glm::vec3(50.0f, 50.0f, 1.f);
    renderer_add_quad(data.renderer, transform);
    
    // Quad 1:
    transform.position = glm::vec3(100.f, 200.f, 0.f);
    transform.scale = glm::vec3(50.0f, 50.0f, 1.f);
    renderer_add_quad(data.renderer, transform, 1);
}

static void update(application_data& data)
{
    renderer_begin(data.renderer);
}

int main()
{
    application_data appData;
    initialize_window(appData);
    initialize_renderer(appData);

    while(!glfwWindowShouldClose(appData.window))
    {
        glfwPollEvents();

        update(appData);

        glfwSwapBuffers(appData.window);
    }

    return 0;
}