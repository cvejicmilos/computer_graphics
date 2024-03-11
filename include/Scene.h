#pragma once

#include <vector>

#include "Maths.h"
#include "Quad.h"
#include "Global.h"

class Shader;
class Model;

struct DepthMapInfo {
    GLuint shadowMapFBO = 0;
    GLuint shadowMapTexture = 0;
    Matrix4 proj, view;
};

struct DirectionalLight {
    float intensity = 1.f;
    Vec3 diffuse = { 1.0f, 1.0f, 1.0f };
    Vec3 specular = { 1.f, 1.f, 0.9f };
    Vec3 direction = { .7f, -1.f, -1.f };
    DepthMapInfo depthMapInfo;
};

struct SpotLight {
    float intensity = 1.0f;
    Vec3 diffuse = { 1.f, 1.f, 1.f };
    Vec3 specular = { 1.f, 1.f, 1.f };
    Vec3 direction = { 0, 0, -1 }; 
    Vec3 position = { 10, 0, 0 };  
    float cutOff = PI32 * .06125f;
    float outerCutOff = PI32 * .06125f * 1.5f;
    DepthMapInfo depthMapInfo;
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

    GLuint m_SkyboxCubemap;
    GLuint m_SkyboxVAO, m_SkyboxVBO;
    GLuint m_QuadVAO, m_QuadVBO; // Quad geometry for debug draw texture

    int m_NextActiveTexture = 0;

    Vec3 m_AmbientColor = Vec3{ 0.15f, 0.15f, 0.15f };
    Vec3 m_AccumulatedDirColor;

    Matrix4 m_ProjMatrix = Matrix4::CreatePerspective(PI32 * 0.4f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 1.f, 700.0f);
    Matrix4 m_ViewMatrix = Matrix4::Identity();   

    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    const float SHADOW_DISTANCE = 100.0f;
    const float SHADOW_NEAR = 1.0f, SHADOW_FAR = 300.f;

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
    void AddGrass(Vec3 pos, float rotation, Vec2 size);

    void SetAmbientColor(const Vec3& color) { m_AmbientColor = color; }
    const Vec3& GetAmbientColor() const { return m_AmbientColor; }

    Matrix4& GetProjectionMatrix() { return m_ProjMatrix; }
    Matrix4& GetViewMatrix() { return m_ViewMatrix; }

    void Draw(Shader& objShader, Shader& skyboxShader, Shader& depthMapShader, Texture grassTexture, Texture grassNormalMap = Texture());

private:
    void DrawSkybox(Shader& shader);
    void UploadLightData(Shader& shader);
    void DrawGeometry(Shader& shader, Matrix4 view, Matrix4 proj, Texture grassTexture, Texture grassNormalMap);
    void DebugDrawTexture(GLuint texture);
    void DrawShadowMap(Shader& shader, DepthMapInfo& info, Texture grassTexture, Texture grassNormalMap);
    void InitLightDepthMap(DepthMapInfo& info);
};