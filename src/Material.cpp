#include <assert.h>
#include <iostream>
#include <fstream>

#include "Material.h"
#include "Utils.h"
#include "Shader.h"

MaterialLibrary* MaterialLibrary::s_Instance = NULL;
MaterialLibrary& MaterialLibrary::Get() {
    if (!s_Instance) s_Instance = new MaterialLibrary();

    return *s_Instance;
}

MaterialLibrary::MaterialLibrary() {
    assert(!s_Instance && "Only one instance allowed");

    s_Instance = this;
}

void MaterialLibrary::LoadMaterialFile(const std::string& path) {
    std::ifstream file(path);
    assert(file.is_open() && "Could not open file");

    std::cout << "Loading materials from '" << path << "...\n";

    Material currentMaterial;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "#") {
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (prefix == "newmtl") {
            if (!currentMaterial.name.empty()) {
                AddMaterial(currentMaterial.name, currentMaterial);
                currentMaterial = Material();
            }
            iss >> currentMaterial.name;

        } else if (prefix == "map_Kd") {
            std::string diffusePath;
            iss >> diffusePath;
            currentMaterial.diffuseMap = loadTexture(sameDirPath(path, diffusePath)); // Relative to mtl path
        } else if (prefix == "map_Ks") {
            std::string specularPath;
            iss >> specularPath;
            currentMaterial.specularMap = loadTexture(sameDirPath(path, specularPath)); // Relative to mtl path
        } else if (prefix == "map_bump") {
            std::string normalPath;
            iss >> normalPath;
            currentMaterial.normalMap = loadTexture(sameDirPath(path, normalPath)); // Relative to mtl path
        } else if (prefix == "map_Ka") {
            std::string ambientPath;
            iss >> ambientPath;
            currentMaterial.ambientMap = loadTexture(sameDirPath(path, ambientPath)); // Relative to mtl path
        } else if (prefix == "Ka") {
            iss >> currentMaterial.ambientColor.x >> currentMaterial.ambientColor.y >> currentMaterial.ambientColor.z;
        } else if (prefix == "Kd") {
            iss >> currentMaterial.diffuseColor.x >> currentMaterial.diffuseColor.y >> currentMaterial.diffuseColor.z;
        } else if (prefix == "Ks") {
            iss >> currentMaterial.specularColor.x >> currentMaterial.specularColor.y >> currentMaterial.specularColor.z;
        } else if (prefix == "Ke") {
            iss >> currentMaterial.emissiveColor.x >> currentMaterial.emissiveColor.y >> currentMaterial.emissiveColor.z;
        } else if (prefix == "Ns") {
            iss >> currentMaterial.specularExponent;
        } else if (prefix == "d") {
            iss >> currentMaterial.alpha;
        } else if (prefix == "Tr") {
            float alphaInverted;
            iss >> alphaInverted;
            currentMaterial.alpha = 1.f - alphaInverted;
        } 
        // Below are ignored for now
        else if (prefix == "Ni") { // Optical Density / Index of refraction
            std::string dummy;
            iss >> dummy; // Ignore
        } else if (prefix == "illum") { // Illumination mode
            std::string dummy;
            iss >> dummy; // Ignore
        } else if (prefix == "Tf") { // Transmission Filter Color
            std::string dummy;
            iss >> dummy >> dummy >> dummy; // Ignore
        } else if(prefix != "") {
            assert(false && "Unhandled field in material file");
        }
    }

    if (!currentMaterial.name.empty()) {
        materials[currentMaterial.name] = currentMaterial;
    }

    std::cout << "Materials loaded OK!\n";
}
Material* MaterialLibrary::GetMaterial(const std::string& name) {
    assert(materials.find(name) != materials.end() && "No such material");
    return &materials.at(name);
}

void MaterialLibrary::AddMaterial(const std::string& name, const Material& material) {
    materials[name] = material;
    std::cout << "Loaded material '" << name << "'\n";
}

bool MaterialLibrary::ExistsMaterial(const std::string& name) {
    return materials.find(name) != materials.end();
}

void setMaterialInShader(Material& material, Shader& shader, int fromActiveTexture) {

    int nextActiveTexture = fromActiveTexture;

    shader.SetFloat("material.alpha", material.alpha);

    shader.SetInt("material.hasDiffuseMap", material.diffuseMap.id != 0);
    shader.SetInt("material.hasSpecularMap", material.specularMap.id != 0);
    shader.SetInt("material.hasAmbientMap", material.ambientMap.id != 0);
    shader.SetInt("material.hasNormalMap", material.normalMap.id != 0);

    shader.SetVec3("material.diffuseColor", material.diffuseColor);
    if (material.diffuseMap.id) {
        int diffuseActiveTexture = nextActiveTexture++;
        shader.SetInt("material.diffuseMap", diffuseActiveTexture);
        TextureBindContext::Overwrite(diffuseActiveTexture, GL_TEXTURE_2D, material.diffuseMap.id);
    }

    shader.SetVec3("material.specularColor", material.specularColor);
    if (material.specularMap.id) {
        int specularActiveTexture = nextActiveTexture++;
        shader.SetInt("material.specularMap", specularActiveTexture);
        TextureBindContext::Overwrite(specularActiveTexture, GL_TEXTURE_2D, material.specularMap.id);
    }

    shader.SetVec3("material.ambientColor", material.ambientColor);
    if (material.ambientMap.id) {
        int ambientActiveTexture = nextActiveTexture++;
        shader.SetInt("material.ambientMap", ambientActiveTexture);
        TextureBindContext::Overwrite(ambientActiveTexture, GL_TEXTURE_2D, material.ambientMap.id);
    }   

    if (material.normalMap.id) {
        int normalActiveTexture = nextActiveTexture++;
        shader.SetInt("material.normalMap", normalActiveTexture);
        TextureBindContext::Overwrite(normalActiveTexture, GL_TEXTURE_2D, material.normalMap.id);
    }   

    shader.SetFloat("material.specularExponent", material.specularExponent);
    shader.SetFloat("material.specularStrength", 0.5f);
    shader.SetFloat("material.reflectiveness", material.reflectiveness);
    shader.SetFloat("material.shouldCastShadow", material.shouldCastShadow);
}