#pragma once

#include <string>
#include <fstream>

#include "Maths.h"
#include "GLutils.h"


struct ShaderSettings {
    int maxNumPointLights = 16;
    int maxNumDirLights = 16;
    int maxNumSpotLights = 16;
};

class Shader {
private:
    GLuint m_Program;
    std::string m_VertPath, m_FragPath; // Stored for hot reloading
    ShaderSettings m_CurrentSettings;

    static GLuint s_LastBound;

public:
    Shader(const std::string& vertSrcPath, const std::string& fragSrcPath, ShaderSettings setting = ShaderSettings());
    ~Shader();

    void Bind();
    void Unbind();

    void HotReload(ShaderSettings settings = ShaderSettings());

    void SetInt(const std::string& field, int value);
    void SetFloat(const std::string& field, float value);
    void SetVec2(const std::string& field, Vec2 value);
    void SetVec3(const std::string& field, Vec3 value);
    void SetVec4(const std::string& field, Vec4 value);
    void SetMat4(const std::string& field, Matrix4 value);

    const ShaderSettings& GetSettings() const { return m_CurrentSettings; }

private:
    GLuint GetLocation(const std::string& name);
    bool Load(ShaderSettings settings);
    void Unload();
    bool ProcessSource(std::string& src, ShaderSettings settings);
};