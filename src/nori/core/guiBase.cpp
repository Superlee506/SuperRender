//
// Created by superqqli on 2021/12/16.
//

#include <nori/core/guiBase.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

NORI_NAMESPACE_BEGIN

GuiBase::GuiBase()
{
}


float GuiBase::getProgress() const
{
    return m_progress;
}

void GuiBase::setProgress(const float &progress)
{
    m_progress = progress;
}

std::string GuiBase::getRenderTime() const
{
    return m_renderedTimeStr;
}

void GuiBase::setRenderedTime(const std::string &renderedTimeStr)
{
    m_renderedTimeStr = renderedTimeStr;
}

void GuiBase::initGui()
{
    CHECK(m_width > 0 && m_height > 0);
    if (!glfwInit())
    {
        LOG(ERROR) << "Failed to initialize GLFW";
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_pWindow = glfwCreateWindow(m_width, m_height, "SuperRender", nullptr, nullptr);
    if (m_pWindow == nullptr)
    {
        glfwTerminate();
        LOG(ERROR) << "Failed to create GLFW window!";
        return;
    }
    glfwMakeContextCurrent(m_pWindow);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    if (glewInit() != GLEW_OK)
    {
        LOG(ERROR) << "Failed to initialize GLEW!";
        return;
    }
    if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
    {
        LOG(ERROR) << "Support for necessary OpenGL extensions missing.";
        return;
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    LOG(INFO) << "OpenGL initialized! Version: " << glGetString(GL_VERSION);
}

void GuiBase::draw()
{
    while (!glfwWindowShouldClose(m_pWindow))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, m_width, m_height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        drawUI();
        ImGui::Render();
        this->drawContent();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }

   this->releaseResource();
}


void GuiBase::releaseResource()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

NORI_NAMESPACE_END
