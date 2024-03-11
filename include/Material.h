#pragma once

#include <string>
#include <map>
#include "GLutils.h"
#include "Maths.h"

class Shader;

struct Material {
    std::string name = "Unnamed";
    Vec3 diffuseColor = { 1.f, 1.f, 1.f };
    Vec3 ambientColor = { 1.f, 1.f, 1.f };
    Vec3 specularColor = { 1.f, 1.f, 1.f };
    Vec3 emissiveColor = { 0.f, 0.f, 0.f };
    float specularExponent = 32.f;
    float alpha = 1.f;
    float reflectiveness = 0.0f;

    Texture diffuseMap; 
    Texture specularMap;
    Texture ambientMap;
    Texture normalMap; 
};

class MaterialLibrary {
private:
    static MaterialLibrary* s_Instance;

    std::map<std::string, Material> materials;
public:
    static MaterialLibrary& Get();

    MaterialLibrary();

    void LoadMaterialFile(const std::string& path);
    Material* GetMaterial(const std::string& name);
    void AddMaterial(const std::string& name, const Material& material);
    bool ExistsMaterial(const std::string& name);
};

void setMaterialInShader(Material& material, Shader& shader, int fromActiveTexture);