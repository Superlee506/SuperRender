//
// Created by superqqli on 2021/12/16.
//

#pragma once
#include <nori/core/common.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

NORI_NAMESPACE_BEGIN
class GuiBase
{
public:
    GuiBase();
    virtual ~GuiBase() = default;
    virtual void draw();
    float getProgress() const;
    void setProgress(const float& progress);
    std::string getRenderTime() const;
    void setRenderedTime(const std::string& renderedTimeStr);

protected:
    virtual void drawUI() = 0;
    virtual void drawContent() = 0;
    virtual void releaseResource();
    virtual void initGui();

protected:
    int m_width; ///With of the window
    int m_height; ///Height of the window
    std::string m_renderTimeString = "";
    GLuint m_texture = GLuint(-1);
    GLuint m_VBO = GLuint(-1);
    GLuint m_VAO = GLuint(-1);
    GLuint m_EBO = GLuint(-1);
    GLFWwindow * m_pWindow = nullptr;
    float m_progress = 0.0f;
    std::string m_renderedTimeStr = "";
    std::string m_windowsName = "";
};
NORI_NAMESPACE_END
