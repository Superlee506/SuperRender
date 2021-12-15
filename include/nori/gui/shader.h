//
// Created by superqqli on 2021/12/15.
//
#pragma once
#include <nori/core/common.h>
#include <GL\glew.h>

NORI_NAMESPACE_BEGIN
class Shader
{
public:
    Shader(const char * vertexCode, const char * fragmentCode);

    void use();

    void setBool(const std::string & name, bool value) const;

    void setInt(const std::string & name, int value) const;

    void setFloat(const std::string & name, float value) const;

private:
    void checkCompileErrors(GLuint shader, std::string type);

private:
    GLuint m_id;
};
NORI_NAMESPACE_END