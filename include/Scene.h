#pragma once

#include <vector>

#include "Maths.h"
#include "Quad.h"

class Shader;
class Model;

struct DirectionalLight {
    float intensity = 0.3f;
    Vec3 diffuse = { 1.0f, 0.8f, 0.2f };
    Vec3 specular = { 1.f, 1.f, 0.9f };
    Vec3 direction = { .7f, -1.f, -1.f };
};

struct SpotLight {
    float intensity = 1.0f;
    Vec3 diffuse = { 1.f, 1.f, 1.f };
    Vec3 specular = { 1.f, 1.f, 1.f };
    Vec3 direction = { 0, 0, -1 }; 
    Vec3 position = { 10, 0, 0 };  
    float cutOff = PI32 * .06125f;
    float outerCutOff = PI32 * .06125f * 1.5f;
};

struct PointLight {
    float intensity = 1.0f;
    Vec3 diffuse = { 1.f, 1.f, 1.f };
    Vec3 specular = { 1.f, 1.f, 1.f };
    Vec3 position = { 9.f, 3.f, -80.f };
    float constant = 0.6f;
    float linear = .05f;
    float quadratic = .025f;
};


class Scene {
private:
    std::vector<PointLight> m_PointLights;
    std::vector<DirectionalLight> m_DirLights;
    std::vector<SpotLight> m_SpotLights;
    std::vector<Model*> m_Models;
    std::vector<GrassQuad> m_Quads;

    Vec3 m_AmbientColor = Vec3{ 1.f, 1.f, 1.f };

    Matrix4 m_ProjMatrix = Matrix4::Perspective(PI32 / 4.f, 1280.0f / 720.0f, 0.1f, 1000.0f);
    Matrix4 m_ViewMatrix = Matrix4::Identity();   

public:
    Scene();
    ~Scene();

    std::vector<PointLight>& GetPointLights() { return m_PointLights; }
    std::vector<DirectionalLight>& GetDirLights() { return m_DirLights; }
    std::vector<SpotLight>& GetSpotLights() { return m_SpotLights; }

    // These should not be stored because vector might change
    // which will make the references point to invalid memory.
    PointLight& GetPointLightAt(size_t index) { return m_PointLights[index]; }
    DirectionalLight& GetDirLightAt(size_t index) { return m_DirLights[index]; }
    SpotLight& GetSpotLightAt(size_t index) { return m_SpotLights[index]; }

    // These return index in repective vector
    size_t AddPointLight(const PointLight& light = PointLight());
    size_t AddDirLight(const DirectionalLight& light = DirectionalLight());
    size_t AddSpotLight(const SpotLight& light = SpotLight());

    Model* AddModel(Model* model);
    void DeleteModel(Model* model);

    void AddQuad(const GrassQuad& quad);

    void SetAmbientColor(const Vec3& color) { m_AmbientColor = color; }
    const Vec3& GetAmbientColor() const { return m_AmbientColor; }

    Matrix4& GetProjectionMatrix() { return m_ProjMatrix; }
    Matrix4& GetViewMatrix() { return m_ViewMatrix; }

    void Draw(Shader& shader, Texture grassTexture, Texture normalMap = Texture());
};