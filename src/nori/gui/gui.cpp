//
// Created by superqqli on 2021/12/15.
//
#include <nori/gui/gui.h>
#include <nori/core/block.h>

#include <imgui.h>
NORI_NAMESPACE_BEGIN

Gui::Gui(const ImageBlock & block) : m_block(block), m_scale(0.5f)
{

    auto Size = m_block.getSize();
    m_width = Size.x();
    m_height = Size.y();
    m_borderSize = m_block.getBorderSize();
    m_windowsName = "SuperRender";
    this->initGui();

    /*Init shader */
    const GLchar * ScreenVertexShaderSource =
            "#version 330 core\n"
            "layout(location = 0) in vec3 position;\n"
            "layout(location = 1) in vec2 uv;\n"
            "out vec2 out_uv;\n"
            "void main()\n"
            "{\n"
            "	gl_Position = vec4(position, 1.0);\n"
            "	out_uv = uv;\n"
            "}";

    const GLchar * ScreenFragmentShaderSource =
            "#version 330\n"
            "uniform sampler2D source;\n"
            "uniform float scale;\n"
            "in vec2 out_uv;\n"
            "out vec4 out_color;\n"
            "float toSRGB(float value)\n"
            "{\n"
            "    if (value < 0.0031308)\n"
            "        return 12.92 * value;\n"
            "    return 1.055 * pow(value, 0.41666) - 0.055;\n"
            "}\n"
            "void main()\n"
            "{\n"
            "    vec4 color = texture(source, out_uv);\n"
            "    color *= scale / color.w;\n"
            "    out_color = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);\n"
            "}";

    m_screenShader.reset(new Shader(ScreenVertexShaderSource, ScreenFragmentShaderSource));

    const GLchar * blockVertexShaderSource =
            "#version 330 core\n"
            "layout(location = 0) in vec3 position;\n"
            "layout(location = 1) in vec3 color;\n"
            "out vec3 out_color;\n"
            "void main()\n"
            "{\n"
            "	gl_Position = vec4(position, 1.0);\n"
            "	out_color = color;\n"
            "}";

    const GLchar * blockFragmentShaderSource =
            "#version 330\n"
            "in vec3 out_color;\n"
            "out vec4 frag_color;\n"
            "void main()\n"
            "{\n"
            "    frag_color = vec4(out_color, 1);\n"
            "}";

    m_blockShader.reset(new Shader(blockVertexShaderSource, blockFragmentShaderSource));
}


void Gui::drawContent()
{
    float scale = std::pow(2.0f, (m_scale - 0.5f) * 20.0f);
    m_block.lock();

    // 1st pass: draw the full screen with  texture data
    bindScreenVertexBuffer();

    glActiveTexture(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, GLint(m_block.cols()));
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT,
            (uint8_t*)(m_block.data()) + (m_borderSize * m_block.cols() + m_borderSize) * sizeof(Color4f)
    );
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_block.unlock();

    m_screenShader->use();
    m_screenShader->setFloat("scale", scale);
    m_screenShader->setInt("source", 0);

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0));
    glBindVertexArray(0);

    // 2st pass: draw the small mark box
    bindBlockVertexBuffer();
    m_blockShader->use();
    glBindVertexArray(m_VAO);
    glDrawElements(GL_LINES, GLsizei(m_renderingBlocks.size() * 8), GL_UNSIGNED_INT, (void*)(0));
    glBindVertexArray(0);
}


void Gui::drawUI()
{
    if (ImGui::Begin("Option", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SliderFloat("Explosure Value", &m_scale, 0.0f, 1.0f);

        static char buffer[64];
        if (1.0f - m_progress > 1e-4f)
        {
            sprintf(buffer, "%.0f%%(%s)", m_progress * 100 + 0.01f, m_renderedTimeStr.c_str());
        }
        else
        {
            sprintf(buffer, "Finished(%s)", m_renderTimeString.c_str());
        }
        ImGui::ProgressBar(m_progress, ImVec2(-1, 0), buffer);
        ImGui::End();
    }
}

void Gui::bindScreenVertexBuffer()
{
    const unsigned int pIndices[] =
            {
                    0, 1, 3,
                    1, 2, 3
            };

    const float pVertices[] =
            {
                    1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
                    1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
                    -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
                    -1.0f,  1.0f, 0.0f,  0.0f, 0.0f
            };

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVertices), pVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Gui::bindBlockVertexBuffer()
{
    static std::vector<float> vertices;
    vertices.clear();
    vertices.reserve(m_renderingBlocks.size() * 8 * 6);
    static std::vector<unsigned int> indices;
    indices.clear();
    indices.reserve(m_renderingBlocks.size() * 4 * 2);
    for (size_t i = 0; i < m_renderingBlocks.size() * 4 * 2; i++)
    {
        indices.push_back((unsigned int)(i));
    }

    float invWidth = 1.0f / m_width;
    float invHeight = 1.0f / m_height;

    for (const ImageBlock * pRenderingBlock : m_renderingBlocks)
    {
        if (pRenderingBlock != nullptr)
        {
            auto size = pRenderingBlock->getSize() ;
            auto offset = pRenderingBlock->getOffset();
            auto borderSize = pRenderingBlock->getBorderSize();

            offset.x() -= borderSize;
            offset.y() -= borderSize;

            size.x() += 2 * borderSize;
            size.y() += 2 * borderSize;

            Vector3f leftTop    (offset.x() * invWidth * 2.0f - 1.0f, (1.0f - offset.y() * invHeight) * 2.0f - 1.0f, 0.0f);
            Vector3f leftBottom (offset.x() * invWidth * 2.0f - 1.0f, (1.0f - (offset.y() + size.y() - 1) * invHeight) * 2.0f - 1.0f, 0.0f);
            Vector3f rightTop   ((offset.x() + size.x() - 1) * invWidth * 2.0f - 1.0f, (1.0f - offset.y() * invHeight) * 2.0f - 1.0f, 0.0f);
            Vector3f rightBottom((offset.x() + size.x() - 1) * invWidth * 2.0f - 1.0f, (1.0f - (offset.y() + size.y() - 1) * invHeight) * 2.0f - 1.0f, 0.0f);

            vertices.push_back(leftTop.x()); vertices.push_back(leftTop.y()); vertices.push_back(leftTop.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(leftBottom.x()); vertices.push_back(leftBottom.y()); vertices.push_back(leftBottom.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(leftBottom.x()); vertices.push_back(leftBottom.y()); vertices.push_back(leftBottom.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(rightBottom.x()); vertices.push_back(rightBottom.y()); vertices.push_back(rightBottom.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(rightBottom.x()); vertices.push_back(rightBottom.y()); vertices.push_back(rightBottom.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(rightTop.x()); vertices.push_back(rightTop.y()); vertices.push_back(rightTop.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(rightTop.x()); vertices.push_back(rightTop.y()); vertices.push_back(rightTop.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

            vertices.push_back(leftTop.x()); vertices.push_back(leftTop.y()); vertices.push_back(leftTop.z());
            vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
        }
    }

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
NORI_NAMESPACE_END