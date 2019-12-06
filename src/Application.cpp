#include "Application.h"
#include "imgui.h"
#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"
#include <stdexcept>

Application::Application()
{
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        throw std::runtime_error("GLFW Init failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "RayTracedChess", NULL, NULL);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Disable vsync

    ge::gl::init((ge::gl::GET_PROC_ADDRESS)glfwGetProcAddress);
    ge::gl::setHighDebugMessage();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    io.Fonts->AddFontFromFileTTF("res/fonts/Roboto-Medium.ttf", 16.0f);

    glViewport(0, 0, width, height);
}

void Application::start()
{
    // Initialize Callbacks
    using namespace std::placeholders;
    Callback<void(GLFWwindow*, int, int)>::func = std::bind(&Application::framebufferSizeCallback, this, _2, _3);
    GLFWframebuffersizefun f1 = static_cast<GLFWframebuffersizefun>(Callback<void(GLFWwindow*, int, int)>::callback);
    glfwSetFramebufferSizeCallback(window, f1);

    Callback<void(GLFWwindow*, double, double)>::func = std::bind(&Application::mouseScrollEvent, this, _2, _3);
    GLFWscrollfun f2 = static_cast<GLFWscrollfun>(Callback<void(GLFWwindow*, double, double)>::callback);
    glfwSetScrollCallback(window, f2);

    Callback<void(GLFWwindow*, double, double)>::func = std::bind(&Application::mouseMoveEvent, this, _2, _3);
    GLFWcursorposfun f3 = static_cast<GLFWcursorposfun>(Callback<void(GLFWwindow*, double, double)>::callback);
    glfwSetCursorPosCallback(window, f3);

    Callback<void(GLFWwindow*, int, int, int)>::func = std::bind(&Application::mouseButtonEvent, this, _2, _3, _4);
    GLFWmousebuttonfun f4 = static_cast<GLFWmousebuttonfun>(Callback<void(GLFWwindow*, int, int, int)>::callback);
    glfwSetMouseButtonCallback(window, f4);

    Callback<void(GLFWwindow*, int, int, int, int)>::func = std::bind(&Application::keyEvent, this, _2, _3, _4, _5);
    GLFWkeyfun f5 = static_cast<GLFWkeyfun>(Callback<void(GLFWwindow*, int, int, int, int)>::callback);
    glfwSetKeyCallback(window, f5);

    init();
    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw();

        glfwPollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    deinit();
}

void Application::swap() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

double Application::getDeltaTime()
{
    return deltaTime;
}

void Application::glfwErrorCallback(int error, const char* description)
{
    std::cerr << "Glfw Error " << error << ': ' << description << std::endl;
}

void Application::framebufferSizeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    this->width = width;
    this->height = height;
    resizeEvent(width, height);
}
