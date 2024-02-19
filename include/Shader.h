#pragma once

#include <string>
#include <fstream>

#include "Maths.h"
#include "GLutils.h"

class Shader {
private:
    GLuint m_Program;

    static GLuint s_LastBound;

public:
    Shader(const std::string& vertSrcPath, const std::string& fragSrcPath);
    ~Shader();

    void Bind();
    void Unbind();

    void SetInt(const std::string& field, int value);
    void SetFloat(const std::string& field, float value);
    void SetVec2(const std::string& field, Vec2 value);
    void SetVec3(const std::string& field, Vec3 value);
    void SetVec4(const std::string& field, Vec4 value);
    void SetMat4(const std::string& field, Matrix4 value);

private:
    GLuint GetLocation(const std::string& name);
};