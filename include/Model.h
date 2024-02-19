#pragma once

#include "Maths.h"
#include <vector>
#include <string>
#include <map>
#include <sstream>

#include <glad/glad.h>

#include "Shader.h"


struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec2 uv;
};
struct Texture {
    GLuint id = 0;
    int width, height, channels;
};
struct Material {
    std::string name = "Unnamed";
    Vec3 diffuseColor = { 1.f, 1.f, 1.f };
    Vec3 ambientColor = { 1.f, 1.f, 1.f };
    Vec3 specularColor = { 1.f, 1.f, 1.f };
    Vec3 emissiveColor = { 0.f, 0.f, 0.f };
    float specularExponent = 0.f;
    float alpha = 1.f;

    Texture diffuseMap; 
    Texture normalMap; 
};

Texture loadTexture(const std::string& path);
void deleteTexture(const std::string& path);

class MaterialLibrary {
private:
    static MaterialLibrary* s_Instance;

    std::map<std::string, Material> materials;
public:
    static MaterialLibrary& Get();

    MaterialLibrary();

    void LoadMaterialFile(const std::string& path);
    Material* GetMaterial(const std::string& name);
};
 

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
public:
    Model(const std::string& filepath);
    ~Model();

    void Draw(Shader& shader, Matrix4 modelTransform, Matrix4 view = Matrix4::CreateTranslation(Vec3{0.0f, 0.0f, -3.0f}), Matrix4 proj = Matrix4::Perspective(3.1415f / 4.f, 1280.0f / 720.0f, 0.1f, 100.0f));
};