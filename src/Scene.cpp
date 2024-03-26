#include "Scene.h"

#include "Shader.h"
#include "Model.h"

#include <assert.h>

Scene::Scene() {
    //
    // Set up skybox
    //
    m_SkyboxCubemap = loadCubemap({
        "assets/textures/skybox/posx.jpg",
        "assets/textures/skybox/negx.jpg",
        "assets/textures/skybox/posy.jpg",
        "assets/textures/skybox/negy.jpg",
        "assets/textures/skybox/posz.jpg",
        "assets/textures/skybox/negz.jpg"
    });
    GL_CALL(glGenVertexArrays(1, &m_SkyboxVAO));
    GL_CALL(glGenBuffers(1, &m_SkyboxVBO));

    GL_CALL(glBindVertexArray(m_SkyboxVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO));

    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), 0));
    GL_CALL(glEnableVertexAttribArray(0));

    float radius = 2.f;

    Vec3 cubemapVertices[] ={
        // Front face
        Vec3(-radius, -radius,  radius),
        Vec3(radius, -radius,  radius),
        Vec3(radius,  radius,  radius),
        Vec3(radius,  radius,  radius),
        Vec3(-radius,  radius,  radius),
        Vec3(-radius, -radius,  radius),

        // Back face
        Vec3(-radius, -radius, -radius),
        Vec3(-radius,  radius, -radius),
        Vec3(radius,  radius, -radius),
        Vec3(radius,  radius, -radius),
        Vec3(radius, -radius, -radius),
        Vec3(-radius, -radius, -radius),

        // Left face
        Vec3(-radius,  radius,  radius),
        Vec3(-radius,  radius, -radius),
        Vec3(-radius, -radius, -radius),
        Vec3(-radius, -radius, -radius),
        Vec3(-radius, -radius,  radius),
        Vec3(-radius,  radius,  radius),

        // Right face
        Vec3(radius,  radius,  radius),
        Vec3(radius, -radius, -radius),
        Vec3(radius,  radius, -radius),
        Vec3(radius, -radius, -radius),
        Vec3(radius,  radius,  radius),
        Vec3(radius, -radius,  radius),

        // Bottom face
        Vec3(-radius, -radius, -radius),
        Vec3(radius, -radius, -radius),
        Vec3(radius, -radius,  radius),
        Vec3(radius, -radius,  radius),
        Vec3(-radius, -radius,  radius),
        Vec3(-radius, -radius, -radius),

        // Top face
        Vec3(-radius,  radius, -radius),
        Vec3(-radius,  radius,  radius),
        Vec3(radius,  radius,  radius),
        Vec3(radius,  radius,  radius),
        Vec3(radius,  radius, -radius),
        Vec3(-radius,  radius, -radius)
    };

    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), cubemapVertices,  GL_STATIC_DRAW));

    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    //
    // Set up debug quad
    //
    GL_CALL(glGenVertexArrays(1, &m_QuadVAO));
    GL_CALL(glGenBuffers(1, &m_QuadVBO));

    GL_CALL(glBindVertexArray(m_QuadVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO));

    GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2) + sizeof(Vec2), 0));
    GL_CALL(glEnableVertexAttribArray(0));

    GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2) + sizeof(Vec2), (const void*)sizeof(Vec2)));
    GL_CALL(glEnableVertexAttribArray(1));

    float quadVertices[] = {
        -1.0f, -1.0f,  0.f, 0.f,
        -1.0f,  1.0f,  0.f, 1.f,
         1.0f,  1.0f,  1.f, 1.f,

         1.0f,  1.0f,  1.f, 1.f,
         1.0f, -1.0f,  1.f, 0.f,
        -1.0f, -1.0f,  0.f, 0.f,
    };
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW));

    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    
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

void Scene::AddGrass(Vec3 pos, float rotation, Vec2 size) {
    AddQuad(GrassQuad({ pos.x,        pos.y + size.y / 2.f - 0.1f, pos.z },        size, { 0, 0.f + rotation, 0 }));
    AddQuad(GrassQuad({ pos.x,        pos.y + size.y / 2.f - 0.1f, pos.z + 0.1f }, size, { 0, PI32 + rotation, 0 }));
    AddQuad(GrassQuad({ pos.x,        pos.y + size.y / 2.f - 0.1f, pos.z },        size, { 0, PI32 / 2.f + rotation, 0 }));
    AddQuad(GrassQuad({ pos.x + 0.1f, pos.y + size.y / 2.f - 0.1f, pos.z },        size, { 0, PI32 / 2.f + PI32 + rotation, 0 }));
}

void Scene::Draw(const DrawContext& ctx) {
    m_NextActiveTexture = 0;
    TextureBindContext::ResetAll();

    //
    // Draw skybox
    //
    ctx.skyboxShader->Bind();
    DrawSkybox(ctx);
    ctx.skyboxShader->Unbind();

    m_NextActiveTexture = 0;
    TextureBindContext::ResetAll();

    //
    // Draw shadowmaps
    //
    for (auto& dirLight : m_DirLights) {
        if (!dirLight.depthMapInfo.cast) continue;
        dirLight.depthMapInfo.proj = Matrix4::CreateOrtho(-SHADOW_DISTANCE, SHADOW_DISTANCE, -SHADOW_DISTANCE, SHADOW_DISTANCE, SHADOW_NEAR, SHADOW_FAR);
        dirLight.depthMapInfo.view = Matrix4::CreateLookAt(dirLight.direction.Invert().Multiply(SHADOW_DISTANCE * 1.5f), 
                                    dirLight.direction, 
                                    Vec3( 0.0f, 1.0f,  0.0f));
        dirLight.depthMapInfo.view.Invert();

        if (dirLight.depthMapInfo.shadowMapFBO == 0) {
            InitLightDepthMap(dirLight.depthMapInfo);
        }
        DrawShadowMap(ctx, dirLight.depthMapInfo);
        m_NextActiveTexture = 0;
        TextureBindContext::ResetAll();
    }
    m_NextActiveTexture = 0;
    TextureBindContext::ResetAll();
    for (auto& spotLight : m_SpotLights) {
        spotLight.depthMapInfo.proj = Matrix4::CreatePerspective(spotLight.outerCutOff * 2.0f, (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, SHADOW_NEAR, SHADOW_FAR);
        spotLight.depthMapInfo.view = Matrix4::CreateLookAt(spotLight.position, 
                                    spotLight.position.Add(spotLight.direction), 
                                    Vec3( 0.0f, 1.0f,  0.0f));
        spotLight.depthMapInfo.view.Invert();
        if (spotLight.depthMapInfo.shadowMapFBO == 0) {
            InitLightDepthMap(spotLight.depthMapInfo);
        }
        DrawShadowMap(ctx, spotLight.depthMapInfo);
        m_NextActiveTexture = 0;
        TextureBindContext::ResetAll();
    }
    for (auto& pointLight : m_PointLights) {
        
        float aspect = (float)SHADOW_WIDTH3D/(float)SHADOW_HEIGHT3D;
        pointLight.depthMapInfo.proj = Matrix4::CreatePerspective(PI32 * 0.5f /*90deg*/, aspect, SHADOW_NEAR, SHADOW_FAR);

        pointLight.depthMapInfo.viewTransforms[0] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3( 1.0, 0.0, 0.0)), Vec3(0.0,-1.0, 0.0));
        pointLight.depthMapInfo.viewTransforms[1] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3(-1.0, 0.0, 0.0)), Vec3(0.0,-1.0, 0.0));
        pointLight.depthMapInfo.viewTransforms[2] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3( 0.0,-1.0, 0.0)), Vec3(0.0, 0.0,-1.0));
        pointLight.depthMapInfo.viewTransforms[3] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3( 0.0, 1.0, 0.0)), Vec3(0.0, 0.0, 1.0));
        pointLight.depthMapInfo.viewTransforms[4] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3( 0.0, 0.0, 1.0)), Vec3(0.0,-1.0, 0.0));
        pointLight.depthMapInfo.viewTransforms[5] = Matrix4::CreateLookAt(pointLight.position, pointLight.position.Add(Vec3( 0.0, 0.0,-1.0)), Vec3(0.0,-1.0, 0.0));

        if (pointLight.depthMapInfo.shadowMapFBO == 0) {
            InitLightDepthMap3D(pointLight.depthMapInfo);
        }
        DrawShadowMap3D(ctx, pointLight.depthMapInfo, pointLight.position);
        m_NextActiveTexture = 0;
        TextureBindContext::ResetAll();
    }
    m_NextActiveTexture = 0;
    TextureBindContext::ResetAll();

    //
    // Draw scene from view
    //
    ctx.objShader->Bind();

    UploadLightData(*ctx.objShader);
    AppWindow* window = GetMainWindow();
    GL_CALL(glViewport(0, 0, window->GetWidth(), window->GetHeight()));
    DrawGeometry(*ctx.objShader, this->GetViewMatrix(), this->GetProjectionMatrix(), ctx);

    ctx.objShader->Unbind();

    m_NextActiveTexture = 0;

    if (g_ShouldDrawDepthMaps) {
        GL_CALL(glViewport(0, 0, window->GetWidth(), window->GetHeight()));

        TextureBindContext::ResetAll();
        if (g_DrawDepthMapIndex < m_DirLights.size()) {
            DebugDrawTexture(m_DirLights[g_DrawDepthMapIndex].depthMapInfo.shadowMapTexture);
        } else if ((g_DrawDepthMapIndex - m_DirLights.size()) < m_SpotLights.size()) {
            DebugDrawTexture(m_SpotLights[g_DrawDepthMapIndex - m_DirLights.size()].depthMapInfo.shadowMapTexture);
        }
        
    }
}

void Scene::DrawSkybox(const DrawContext& ctx) {

    // Need to see inside of faces of skybox cube
    GL_CALL(glViewport(0, 0, GetMainWindow()->GetWidth(), GetMainWindow()->GetHeight()));
    GL_CALL(glCullFace(GL_FRONT));
    GL_CALL(glDepthMask(GL_FALSE));

    GL_CALL(glBindVertexArray(m_SkyboxVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO));

    TextureBindContext::Set(0, GL_TEXTURE_CUBE_MAP, m_SkyboxCubemap);

    m_AccumulatedDirColor = { 0, 0, 0 };
    for (size_t i = 0; i < this->GetDirLights().size(); i++ ) {
        auto& light = this->GetDirLights()[i];
        m_AccumulatedDirColor = m_AccumulatedDirColor.Add(light.diffuse.Multiply(light.intensity));
    }

    Matrix4 viewCopy = m_ViewMatrix;
    viewCopy.SetTranslation(Vec3{ 0, 0, 0 });
    viewCopy.Invert();
    ctx.skyboxShader->SetMat4("projection", m_ProjMatrix);
    ctx.skyboxShader->SetMat4("view", viewCopy);
    ctx.skyboxShader->SetVec3("ambientColor", m_AccumulatedDirColor);
    ctx.skyboxShader->SetInt("skybox", 0);

    TextureBindContext::ApplyAll();

    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 36));
    GL_CALL(glDepthMask(GL_TRUE));
    GL_CALL(glCullFace(GL_BACK));
}

void Scene::UploadLightData(Shader& shader) {
    // If num lights are more than the generated constant
    // in the shaders from shader settings, then we need
    // to recompile the shader to support more lights.
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
        shader.SetFloat(fullString + ".innerRadius", light.innerRadius);
        shader.SetFloat(fullString + ".outerRadius", light.outerRadius);
        

        if (light.depthMapInfo.cast && light.depthMapInfo.shadowCubeMap) {
            int lightActiveTexture = m_NextActiveTexture++;
            TextureBindContext::Set(lightActiveTexture, GL_TEXTURE_CUBE_MAP, light.depthMapInfo.shadowCubeMap);
            shader.SetInt(fullString + ".shadowMap", lightActiveTexture);
        }
        shader.SetInt(fullString + ".shouldCast", (int)light.depthMapInfo.cast);
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

        if (light.depthMapInfo.cast && light.depthMapInfo.shadowMapFBO) {
            int lightActiveTexture = m_NextActiveTexture++;
            TextureBindContext::Set(lightActiveTexture, GL_TEXTURE_2D, light.depthMapInfo.shadowMapTexture);
            shader.SetInt(fullString + ".depthMapInfo.shadowMap", lightActiveTexture);
            shader.SetMat4(fullString + ".depthMapInfo.proj", light.depthMapInfo.proj);
            auto viewInverse = light.depthMapInfo.view;
            viewInverse.Invert();
            shader.SetMat4(fullString + ".depthMapInfo.view", viewInverse);
        }
        shader.SetInt(fullString + ".depthMapInfo.shouldCast", light.depthMapInfo.cast);
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

        if (light.depthMapInfo.cast && light.depthMapInfo.shadowMapFBO) {
            int lightActiveTexture = m_NextActiveTexture++;
            TextureBindContext::Set(lightActiveTexture, GL_TEXTURE_2D, light.depthMapInfo.shadowMapTexture);
            shader.SetInt(fullString + ".depthMapInfo.shadowMap", lightActiveTexture);
            auto viewInverse = light.depthMapInfo.view;
            shader.SetMat4(fullString + ".depthMapInfo.proj", light.depthMapInfo.proj);
            viewInverse.Invert();
            shader.SetMat4(fullString + ".depthMapInfo.view", viewInverse);
        }
        shader.SetInt(fullString + ".depthMapInfo.shouldCast", light.depthMapInfo.cast);
    }
    shader.SetInt("numSpotLights", (int)this->GetSpotLights().size());
}

void Scene::DrawGeometry(Shader& shader, Matrix4 view, Matrix4 proj, const DrawContext& ctx) {
    Matrix4 viewInverse = view;
    viewInverse.Invert();
    shader.SetMat4("view", viewInverse);
    shader.SetMat4("projection", proj);

    shader.SetVec3("ambientColor", this->GetAmbientColor());
    shader.SetVec3("skyboxAmbient", m_AccumulatedDirColor);
    shader.SetFloat("shadowFarPlane", SHADOW_FAR);

    int skyboxActiveTexture = m_NextActiveTexture++;
    TextureBindContext::Set(skyboxActiveTexture, GL_TEXTURE_CUBE_MAP, m_SkyboxCubemap);
    shader.SetInt("skybox", skyboxActiveTexture);

    for (auto model : m_Models) {
        model->Draw(*this, shader, m_NextActiveTexture);
    }

    if (m_Quads.size() > 0) batchDrawGrass(m_Quads, shader, ctx.grassTexture, ctx.grassNormalMap, m_NextActiveTexture);
}

void Scene::DebugDrawTexture(GLuint texture) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    Shader& shader = Shader::Basic2D();


    TextureBindContext::Set(0, GL_TEXTURE_2D, texture);

    shader.Bind();
    shader.SetInt("theTexture", 0);

    GL_CALL(glBindVertexArray(m_QuadVAO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO));

    TextureBindContext::ApplyAll();
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

    shader.Unbind();

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
void Scene::DrawShadowMap(const DrawContext& ctx, DepthMapInfo& info) {
    GL_CALL(glCullFace(GL_FRONT));
    ctx.depthMapShader->Bind();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, info.shadowMapFBO));
    GL_CALL(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
    DrawGeometry(*ctx.depthMapShader, info.view, info.proj, ctx);
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    ctx.depthMapShader->Unbind();
    GL_CALL(glCullFace(GL_BACK));
}
void Scene::DrawShadowMap3D(const DrawContext& ctx, DepthMapInfo3D& info, Vec3 lightPos) {
    GL_CALL(glCullFace(GL_FRONT));
    ctx.depthMapShader3D->Bind();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, info.shadowMapFBO));
    GL_CALL(glViewport(0, 0, SHADOW_WIDTH3D, SHADOW_HEIGHT3D));
    GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
    ctx.depthMapShader3D->SetMat4Array("lightTransforms", info.viewTransforms, 6);
    ctx.depthMapShader3D->SetMat4("lightProj", info.proj);
    ctx.depthMapShader3D->SetVec3("lightPos", lightPos);
    ctx.depthMapShader3D->SetFloat("farPlane", SHADOW_FAR);
    DrawGeometry(*ctx.depthMapShader3D, Matrix4::Identity(), Matrix4::Identity(), ctx);
    GL_CALL(glCullFace(GL_BACK));
    ctx.depthMapShader3D->Unbind();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));


    /*
    // Ultra slow sanity checking the shadow map
    glBindFramebuffer(GL_FRAMEBUFFER, info.shadowMapFBO);

    float* pixels = new float[SHADOW_WIDTH3D * SHADOW_HEIGHT3D];

    for (int face = 0; face < 6; ++face) {
        glReadPixels(0, 0, SHADOW_WIDTH3D, SHADOW_HEIGHT3D, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

        for (int x = 0; x < (int)SHADOW_WIDTH3D; x++) {
            for (int y = 0; y < (int)SHADOW_HEIGHT3D; y++) {
                float px = pixels[(y * SHADOW_WIDTH3D) + x];
                if (px < 0.0 || px > 1.0) {
                    CRASH();
                }
            }   
        }
    }

    delete[] pixels;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    */
}
void Scene::InitLightDepthMap(DepthMapInfo& info) {

    std::cout << "Creating Light Depth Map...\n";

    GL_CALL(glGenFramebuffers(1, &info.shadowMapFBO));

    GL_CALL(glGenTextures(1, &info.shadowMapTexture));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, info.shadowMapTexture));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)); 
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)); 

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, info.shadowMapFBO));
    // Attach the texture as a depth attachment
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, info.shadowMapTexture, 0));
    GL_CALL(glDrawBuffer(GL_NONE)); // No color buffer is drawn to
    GL_CALL(glReadBuffer(GL_NONE)); // No color buffer is read from
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0)); 

    std::cout << "Done!\n";
}

void Scene::InitLightDepthMap3D(DepthMapInfo3D& info) {
    std::cout << "Creating Light Depth Map 3D...\n";
    GL_CALL(glGenTextures(1, &info.shadowCubeMap));
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, info.shadowCubeMap));
    for (unsigned int i = 0; i < 6; ++i) {
        GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
                     SHADOW_WIDTH3D, SHADOW_HEIGHT3D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
    }

    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    GL_CALL(glGenFramebuffers(1, &info.shadowMapFBO));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, info.shadowMapFBO));
    GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, info.shadowCubeMap, 0));
    GL_CALL(glDrawBuffer(GL_NONE));
    GL_CALL(glReadBuffer(GL_NONE));

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}