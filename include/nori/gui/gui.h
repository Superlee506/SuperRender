//
// Created by superqqli on 2021/12/15.
//
#pragma once

#include <nori/core/common.h>
#include <nori/gui/shader.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

NORI_NAMESPACE_BEGIN
class Gui
{
public:
    Gui(const ImageBlock & block);
    std::vector<const ImageBlock *> & getRenderingBlocks();
    float & getProgress();
    std::string & getRenderTimeString();
    void draw();
    void drawUI();

private:
    void bindScreenVertexBuffer();
    void bindBlockVertexBuffer();

private:
    const ImageBlock & m_block;
    GLFWwindow * m_pWindow = nullptr;
    GLuint m_texture = GLuint(-1);
    GLuint m_VBO = GLuint(-1);
    GLuint m_VAO = GLuint(-1);
    GLuint m_EBO = GLuint(-1);
    std::unique_ptr<Shader> m_screenShader = nullptr;
    std::unique_ptr<Shader> m_blockShader = nullptr;
    int m_width;
    int m_height;
    int m_borderSize;
    float m_scale;
    std::vector<const ImageBlock *> m_renderingBlocks;
    float m_progress = 0.0f;
    std::string m_renderTimeString = "";
};
NORI_NAMESPACE_END