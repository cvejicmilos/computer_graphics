#include "Shader.h"

#include <iostream>
#include <assert.h>

#include <filesystem>
namespace fs = std::filesystem;

GLuint Shader::s_LastBound = 0;

std::string g_FallbackVertPath = "assets/shaders/basic.vert";
std::string g_FallbackFragPath = "assets/shaders/basic.frag";
Shader* g_FallbackShader = NULL;

std::string g_2DVertPath = "assets/shaders/basic2d.vert";
std::string g_2DFragPath = "assets/shaders/basic2d.frag";
Shader* g_2DShader = NULL;

std::string readFileString(const std::string& path) {
    std::ifstream file(path);
    assert(file.is_open() && "Failed to open file");

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

Shader::Shader(const std::string& vertSrcPath, const std::string& fragSrcPath, const std::string& geoSrcPath, ShaderSettings settings)
    : m_HasGeoShader(fs::exists(geoSrcPath)) {
    m_VertPath = vertSrcPath;
    m_FragPath = fragSrcPath;
    m_GeoPath = geoSrcPath;
    Load(settings); // If fail, fallback shader will be used
}
Shader::~Shader() {
    Unload();
}

void Shader::Bind() {
    GL_CALL(glUseProgram(m_Program));
    s_LastBound = m_Program;
}
void Shader::Unbind() {
    GL_CALL(glUseProgram(0));
    s_LastBound = 0;
}

void Shader::HotReload(ShaderSettings settings) {
    std::cout << "Attempting hot reload on shader\n";

    bool result = Load(settings);

    if (!result) {
        std::cerr << "Hot reload failed; fallback basic shader.\n";
    }
}

void Shader::SetInt(const std::string& field, int value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform1i(GetLocation(field), value));
}
void Shader::SetIntArray(const std::string& field, int* values, int count) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform1iv(GetLocation(field), count, values));
}
void Shader::SetFloat(const std::string& field, float value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform1f(GetLocation(field), value));
}
void Shader::SetVec2(const std::string& field, Vec2 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform2f(GetLocation(field), value.x, value.y));
}
void Shader::SetVec3(const std::string& field, Vec3 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform3f(GetLocation(field), value.x, value.y, value.z));
}
void Shader::SetVec4(const std::string& field, Vec4 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniform4f(GetLocation(field), value.x, value.y, value.z, value.w));
}
void Shader::SetMat4(const std::string& field, Matrix4 value) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniformMatrix4fv(GetLocation(field), 1, GL_FALSE, value.data));
}
void Shader::SetMat4Array(const std::string& field, Matrix4* values, int count) {
    assert(s_LastBound == m_Program && "Shader is not bound");
    GL_CALL(glUniformMatrix4fv(GetLocation(field), (GLsizei)count, GL_FALSE, (GLfloat*)values));
}

GLuint Shader::GetLocation(const std::string& name) {
    GL_CALL(auto loc = glGetUniformLocation(m_Program, name.c_str()));
    return loc;
}

bool Shader::Load(ShaderSettings settings) {

    assert(settings.maxNumDirLights > 0 && settings.maxNumPointLights > 0 && settings.maxNumSpotLights > 0);

    m_CurrentSettings = settings;

    std::cout << "Loading shader from '" << m_VertPath << "' and '" << m_FragPath << "'\n";

    GLuint vertexShader;
    std::string vertSrc = readFileString(m_VertPath);
    if (!ProcessSource(vertSrc, settings)) return false;
    GL_CALL(vertexShader = glCreateShader(GL_VERTEX_SHADER));
    const GLchar* vertSrcPtr = vertSrc.c_str();
    GL_CALL(glShaderSource(vertexShader, 1, &vertSrcPtr, NULL));
    GL_CALL(glCompileShader(vertexShader));


    GLuint fragmentShader;
    std::string fragSrc = readFileString(m_FragPath);
    if (!ProcessSource(fragSrc, settings)) return false;
    const GLchar* fragSrcPtr = fragSrc.c_str();
    GL_CALL(fragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CALL(glShaderSource(fragmentShader, 1, &fragSrcPtr, NULL));
    GL_CALL(glCompileShader(fragmentShader));

    GLuint geoShader = 0;
    if (m_HasGeoShader) {
        std::cout << "Has geometry shader: " << m_GeoPath << "\n";
        std::string geoSrc = readFileString(m_GeoPath);
        if (!ProcessSource(geoSrc, settings)) return false;
        GL_CALL(geoShader = glCreateShader(GL_GEOMETRY_SHADER));
        const GLchar* geoSrcPtr = geoSrc.c_str();
        GL_CALL(glShaderSource(geoShader, 1, &geoSrcPtr, NULL));
        GL_CALL(glCompileShader(geoShader));
    }


    bool anyError = false;

    GLint success;
    GLchar infoLog[512];

    GL_CALL(glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL_CALL(glGetShaderInfoLog(vertexShader, 512, NULL, infoLog));
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        anyError = true;
    }

    GL_CALL(glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success));
    if(!success) {
        GL_CALL(glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog));
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        anyError = true;
    }

    if (m_HasGeoShader) {
        GL_CALL(glGetShaderiv(geoShader, GL_COMPILE_STATUS, &success));
        if(!success) {
            GL_CALL(glGetShaderInfoLog(geoShader, 512, NULL, infoLog));
            std::cerr << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
            anyError = true;
        }
    }

    if (anyError) {
        GL_CALL(glDeleteShader(vertexShader));
        GL_CALL(glDeleteShader(fragmentShader));
        if (m_HasGeoShader) { GL_CALL(glDeleteShader(geoShader)); }
    } else {
        GL_CALL(m_Program = glCreateProgram());
        GL_CALL(glAttachShader(m_Program, vertexShader));
        GL_CALL(glAttachShader(m_Program, fragmentShader));
        if (m_HasGeoShader) { GL_CALL(glAttachShader(m_Program, geoShader)); }
        GL_CALL(glLinkProgram(m_Program));

        GL_CALL(glGetProgramiv(m_Program, GL_LINK_STATUS, &success));
        if(!success) {
            GL_CALL(glGetProgramInfoLog(m_Program, 512, NULL, infoLog));
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            GL_CALL(glDeleteProgram(m_Program));
            anyError = true;
        }

        GL_CALL(glDeleteShader(vertexShader));
        GL_CALL(glDeleteShader(fragmentShader));
        if (m_HasGeoShader) { GL_CALL(glDeleteShader(geoShader)); }
    }

    if (!anyError) {
        std::cout << "Shader Loading OK!\n";
    } else {
        std::cout << "Fallback shader used\n";

        m_Program = Shader::Basic().m_Program;
    }

    return !anyError;
}
void Shader::Unload() {
    GL_CALL(glDeleteProgram(m_Program));
}

void tryReplaceAllInString(std::string& str, const std::string& oldSubstr, const std::string& newSubstr) {
    size_t pos = str.find(oldSubstr);
    while (pos != std::string::npos) {
        str.replace(pos, std::string(oldSubstr).length(), newSubstr);
        pos = str.find(oldSubstr);
    }
}

bool Shader::ProcessSource(std::string& src, ShaderSettings settings) {

    tryReplaceAllInString(src, "##MAX_NUM_POINTLIGHTS", std::to_string(settings.maxNumPointLights));
    tryReplaceAllInString(src, "##MAX_NUM_DIRLIGHTS", std::to_string(settings.maxNumDirLights));
    tryReplaceAllInString(src, "##MAX_NUM_SPOTLIGHTS", std::to_string(settings.maxNumSpotLights));

    return true;
}

Shader& Shader::Basic() {
    if (!g_FallbackShader) {
        g_FallbackShader = new Shader(g_FallbackVertPath, g_FallbackFragPath);
    }
    return *g_FallbackShader;
}

Shader& Shader::Basic2D() {
    if (!g_2DShader) {
        g_2DShader = new Shader(g_2DVertPath, g_2DFragPath);
    }
    return *g_2DShader;
}