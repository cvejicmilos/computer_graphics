#include "Scene.h"

#include "Shader.h"
#include "Model.h"

Scene::Scene() {

}
Scene::~Scene() {

}

size_t Scene::AddPointLight(const PointLight& light) {
    m_PointLights.push_back(light);
    return m_PointLights.size() -1;
}
size_t Scene::AddDirLight(const DirectionalLight& light) {
    m_DirLights.push_back(light);
    return m_DirLights.size() -1;
}
size_t Scene::AddSpotLight(const SpotLight& light) {
    m_SpotLights.push_back(light);
    return m_SpotLights.size() -1;
}

Model* Scene::AddModel(Model* model) {
    m_Models.push_back(model);

    return model;
}
void Scene::DeleteModel(Model* model) {
    // Unsorted Erase so I don't need to shift memory

    for (size_t i = 0; i < m_Models.size(); i++) {
        if (m_Models[i] == model) {
            delete m_Models[i];
            if (m_Models.size() > 1) m_Models[i] = m_Models.back();
            m_Models.pop_back();
            break;
        }
    }
}

void Scene::AddQuad(const GrassQuad& quad) {
    m_Quads.push_back(quad);
}

void Scene::Draw(Shader& shader, Texture grassTexture, Texture normalMap) {
    ShaderSettings newSettings = shader.GetSettings();
    bool needReload = false;
    if (shader.GetSettings().maxNumPointLights < this->GetPointLights().size()) {
        newSettings.maxNumPointLights = (int)this->GetPointLights().size();
        needReload = true;
    }
    if (shader.GetSettings().maxNumDirLights < this->GetDirLights().size()) {
        newSettings.maxNumDirLights = (int)this->GetDirLights().size();
        needReload = true;
    }
    if (shader.GetSettings().maxNumSpotLights < this->GetSpotLights().size()) {
        newSettings.maxNumPointLights = (int)this->GetSpotLights().size();
        needReload = true;
    }
    if (needReload) {
        shader.HotReload(newSettings);
    }

    shader.Bind();

    Matrix4 view = this->GetViewMatrix();
    Matrix4 viewInverse = view;
    viewInverse.Invert();
    shader.SetMat4("view", viewInverse);
    shader.SetMat4("projection", this->GetProjectionMatrix());

    shader.SetVec3("ambientColor", this->GetAmbientColor());
    

    // We don't want to go too crazy creating insane amounts of
    // strings here becaues this happens every frame.
    std::string uniformString;
    std::string subScriptString;
    std::string fullString;

    uniformString = "pointLights";
    for (size_t i = 0; i < this->GetPointLights().size(); i++ ) {
        subScriptString = "[" + std::to_string(i) + "]";
        fullString = uniformString + subScriptString;

        auto& light = this->GetPointLights()[i];

        shader.SetFloat(fullString + ".intensity", light.intensity);
        shader.SetVec3(fullString + ".diffuse", light.diffuse);
        shader.SetVec3(fullString + ".specular", light.specular);
        shader.SetVec3(fullString + ".position", light.position);
        shader.SetFloat(fullString + ".constant", light.constant);
        shader.SetFloat(fullString + ".linear", light.linear);
        shader.SetFloat(fullString + ".quadratic", light.quadratic);
    }
    shader.SetInt("numPointLights", (int)this->GetPointLights().size());

    uniformString = "dirLights";
    for (size_t i = 0; i < this->GetDirLights().size(); i++ ) {
        subScriptString = "[" + std::to_string(i) + "]";
        fullString = uniformString + subScriptString;

        auto& light = this->GetDirLights()[i];

        shader.SetFloat(fullString + ".intensity", light.intensity);
        shader.SetVec3(fullString + ".diffuse", light.diffuse);
        shader.SetVec3(fullString + ".specular", light.specular);
        shader.SetVec3(fullString + ".direction", light.direction);
    }
    shader.SetInt("numDirLights", (int)this->GetDirLights().size());

    uniformString = "spotLights";
    for (size_t i = 0; i < this->GetSpotLights().size(); i++ ) {
        subScriptString = "[" + std::to_string(i) + "]";
        fullString = uniformString + subScriptString;

        auto& light = this->GetSpotLights()[i];

        shader.SetFloat(fullString + ".intensity", light.intensity);
        shader.SetVec3(fullString + ".diffuse", light.diffuse);
        shader.SetVec3(fullString + ".specular", light.specular);
        shader.SetVec3(fullString + ".direction", light.direction);
        shader.SetVec3(fullString + ".position", light.position);
        shader.SetFloat(fullString + ".cutOff", cos(light.cutOff));
        shader.SetFloat(fullString + ".outerCutOff", cos(light.outerCutOff));
    }
    shader.SetInt("numSpotLights", (int)this->GetSpotLights().size());

    for (auto model : m_Models) {
        model->Draw(*this, shader);
    }

    if (m_Quads.size() > 0) batchDrawGrass(m_Quads, shader, grassTexture, normalMap);

    shader.Unbind();
}