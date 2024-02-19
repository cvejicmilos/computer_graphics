#include "Shader.h"

#include <iostream>
#include <assert.h>

GLuint Shader::s_LastBound = 0;

std::string readFileString(const std::string& path) {
    std::ifstream file(path);
    assert(file.is_open() && "Failed to open file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

Shader::Shader(const std::string& vertSrcPath, const std::string& fragSrcPath) {

    std::string vertSrc = readFileString(vertSrcPath);
    std::string fragSrc = readFileString(fragSrcPath);

    GLuint vertexShader, fragmentShader;
    GL_CALL(vertexShader = glCreateShader(GL_VERTEX_SHADER));

    const GLchar* vertSrcPtr = vertSrc.c_str();
    GL_CALL(glShaderSource(vertexShader, 1, &vertSrcPtr, NULL));
    GL_CALL(glCompileShader(vertexShader));

    const GLchar* fragSrcPtr = fragSrc.c_str();
    GL_CALL(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fragmentShader, 1, &fragSrcPtr, NULL));
    GL_CALL(glCompileShader(fragmentShader));

    GLint success;
    GLchar infoLog[512];

    GL_CALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL_CALL(glGetShaderInfoLog(vertexShader, 512, NULL, infoLog));
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GL_CALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL_CALL(glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog));
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GL_CALL(m_Program = glCreateProgram());
    GL_CALL(glAttachShader(m_Program, vertexShader));
    GL_CALL(glAttachShader(m_Program, fragmentShader));
    GL_CALL(glLinkProgram(m_Program));

    GL_CALL(glGetProgramiv(m_Program, GL_LINK_STATUS, &success));
    if(!success) {
        GL_CALL(glGetProgramInfoLog(m_Program, 512, NULL, infoLog));
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    GL_CALL(glDeleteShader(vertexShader));
    GL_CALL(glDeleteShader(fragmentShader));
}
Shader::~Shader() {
    GL_CALL(glDeleteProgram(m_Program));    
}

void Shader::Bind() {
    GL_CALL(glUseProgram(m_Program));
    s_LastBound = m_Program;
}
void Shader::Unbind() {
    GL_CALL(glUseProgram(0));
    s_LastBound = 0;
}

void Shader::SetInt(const std::string& field, int value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniform1i(GetLocation(field), value);
}
void Shader::SetFloat(const std::string& field, float value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniform1f(GetLocation(field), value);
}
void Shader::SetVec2(const std::string& field, Vec2 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniform2f(GetLocation(field), value.x, value.y);
}
void Shader::SetVec3(const std::string& field, Vec3 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniform3f(GetLocation(field), value.x, value.y, value.z);
}
void Shader::SetVec4(const std::string& field, Vec4 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniform4f(GetLocation(field), value.x, value.y, value.z, value.w);
}
void Shader::SetMat4(const std::string& field, Matrix4 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    glUniformMatrix4fv(GetLocation(field), 1, GL_FALSE, value.data);
}

GLuint Shader::GetLocation(const std::string& name) {
    GL_CALL(return glGetUniformLocation(m_Program, name.c_str()));
}