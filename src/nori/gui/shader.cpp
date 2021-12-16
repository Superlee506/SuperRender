//
// Created by superqqli on 2021/12/15.
//
#include <nori/gui/shader.h>

NORI_NAMESPACE_BEGIN
Shader::Shader(const char * vertexCode, const char * fragmentCode)
{
    GLuint vs, fs;

    // vertex shader
    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexCode, NULL);
    glCompileShader(vs);
    checkCompileErrors(vs, "Vertex");

    // fragment Shader
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentCode, NULL);
    glCompileShader(fs);
    checkCompileErrors(fs, "Fragment");

    // shader Program
    m_id = glCreateProgram();
    glAttachShader(m_id, vs);
    glAttachShader(m_id, fs);

    glLinkProgram(m_id);
    checkCompileErrors(m_id, "Program");

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vs);
    glDeleteShader(fs);
}

void Shader::use()
{
    glUseProgram(m_id);
}

void Shader::setBool(const std::string & name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), int(value));
}

void Shader::setInt(const std::string & name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::setFloat(const std::string & name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "Program")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            LOG(ERROR) << "Shader complie error : " << type << "\n" << infoLog;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            LOG(ERROR) << "Program link error : " << type << "\n" << infoLog;
        }
    }
}
NORI_NAMESPACE_END
