//
// Created by superqqli on 2021/12/15.
//
#pragma once

#include <nori/core/common.h>
#include <nori/core/guiBase.h>
#include <nori/gui/shader.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

NORI_NAMESPACE_BEGIN
class Gui: public GuiBase
{
public:
    Gui(const ImageBlock & block);

protected:
    virtual void drawUI() override;
    virtual void drawContent() override;

private:
    void bindScreenVertexBuffer();
    void bindBlockVertexBuffer();

private:
    const ImageBlock & m_block;
    std::unique_ptr<Shader> m_screenShader = nullptr;
    std::unique_ptr<Shader> m_blockShader = nullptr;
    int m_borderSize;
    float m_scale;
    std::vector<const ImageBlock *> m_renderingBlocks;

};
NORI_NAMESPACE_END