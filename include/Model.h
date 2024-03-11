#pragma once

#include "Maths.h"
#include <vector>
#include <string>
#include <map>
#include <sstream>

#include <glad/glad.h>

#include "Shader.h"
#include "GLutils.h"
#include "Material.h"

#include "Utils.h"

class Scene;


 

class Mesh {
private:
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indicies;
    GLuint m_VBO, m_VAO, m_EBO;
    std::string m_Name;
    std::string m_MaterialName;
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::string name, std::string materialName);
    Mesh() {}
    ~Mesh();

    void DrawCall();

    Material* GetMaterialPtr() const { return MaterialLibrary::Get().GetMaterial(m_MaterialName); }
    
};
class Model {
private:
    std::vector<Mesh*> m_Meshes;
    Matrix4 m_Transform = Matrix4::Identity();
public:
    Model(const std::string& filepath);
    ~Model();

    void Draw(Scene& scene, Shader& shader, int fromActiveTexture);
    Matrix4& GetTransform() { return m_Transform; }
};