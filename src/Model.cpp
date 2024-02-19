#include <fstream>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;

#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::string sameDirPath(const std::string& path, const std::string& otherFile) {
    std::string dir = fs::path(path).parent_path().string();
    return dir + "/" + otherFile;
}

// Helper to load GL texture
Texture loadTexture(const std::string& path) {
    Texture texture;

    void* data = stbi_load(path.c_str(), &texture.width, &texture.height, &texture.channels, 4);

    assert(data && "Texture loading failed");

    GL_CALL(glGenTextures(1, &texture.id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture.id));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    texture.channels = 4;

    stbi_image_free(data);

    return texture;
}
void deleteTexture(const Texture& texture) {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture.id));
    GL_CALL(glDeleteTextures(1, &texture.id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

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
                materials[currentMaterial.name] = currentMaterial;
                currentMaterial = Material();
            }
            iss >> currentMaterial.name;

        } else if (prefix == "map_Kd") {
            std::string diffusePath;
            iss >> diffusePath;
            currentMaterial.diffuseMap = loadTexture(sameDirPath(path, diffusePath)); // Relative to mtl path
        } else if (prefix == "map_bump") {
            std::string normalPath;
            iss >> normalPath;
            currentMaterial.normalMap = loadTexture(sameDirPath(path, normalPath)); // Relative to mtl path
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
        } else if (prefix == "Ni") {
            std::string dummy;
            iss >> dummy; // Ignore
        } else if (prefix == "illum") {
            std::string dummy;
            iss >> dummy; // Ignore
        } else if(prefix != "") {
            assert(false && "Unhandled field in material file");
        }
    }

    if (!currentMaterial.name.empty()) {
        materials[currentMaterial.name] = currentMaterial;
    }
}
Material* MaterialLibrary::GetMaterial(const std::string& name) {
    assert(materials.find(name) != materials.end() && "No such material");
    return &materials.at(name);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::string name, std::string materialName) {

    m_Vertices = vertices;
    m_Indicies = indices;
    m_Name = name;
    m_MaterialName = materialName;

    GL_CALL(glGenVertexArrays(1, &m_VAO));
    GL_CALL(glBindVertexArray(m_VAO));

    GL_CALL(glGenBuffers(1, &m_VBO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW));

    GL_CALL(glGenBuffers(1, &m_EBO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indicies.size() * sizeof(unsigned int), &m_Indicies[0], GL_STATIC_DRAW));

    GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos)));
    GL_CALL(glEnableVertexAttribArray(0));

    GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)));
    GL_CALL(glEnableVertexAttribArray(1));

    GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)));
    GL_CALL(glEnableVertexAttribArray(2));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
Mesh::~Mesh() {
    GL_CALL(glDeleteVertexArrays(1, &m_VAO));
    GL_CALL(glDeleteBuffers(1, &m_VBO));
    GL_CALL(glDeleteBuffers(1, &m_EBO));
}

void Mesh::DrawCall() {
    GL_CALL(glBindVertexArray(m_VAO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
    GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indicies.size(), GL_UNSIGNED_INT, 0));

    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
}

Model::Model(const std::string& objPath) {
    std::ifstream file(objPath);
    assert(file.is_open() && "Failed to open file");

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;

    std::vector<Vertex> currentVertices; // Vertices for current mesh
    std::vector<unsigned int> currentIndices;
    std::string currentMeshName = "UNNAMED";
    std::string currentMaterialName = "UNNAMED";
    bool normalSmoothing = true;


    /////// Parse OBJ one object at a time
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "#") {
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (prefix == "s") {
            std::string value;
            iss >> value;
            normalSmoothing = value != "off";
        } else if (prefix == "o") {
            normalSmoothing = true; // reset for each object

            if (currentVertices.size() > 0) {
                m_Meshes.push_back(new Mesh(currentVertices, currentIndices, currentMeshName, currentMaterialName));

                currentVertices.clear();
                currentIndices.clear();
            }

            iss >> currentMeshName;

        } else if (prefix == "mtllib") {
            std::string path;
            iss >> path;
            std::cout << fs::path(objPath).parent_path() << "\n";
            MaterialLibrary::Get().LoadMaterialFile(sameDirPath(objPath, path));
        } else if (prefix == "usemtl") {
            std::string name;
            iss >> name;
            currentMaterialName = name;
        } else if (prefix == "v") {
            Vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            positions.push_back(pos);
        } else if (prefix == "vt") {
            Vec2 uv;
            iss >> uv.x >> uv.y;
            uvs.push_back(uv);
        } else if (prefix == "vn") {
            Vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (prefix == "f") {
            std::string vertexSpec;
            std::vector<Vertex> faceVertices;
            
            // Read all vertices of the face
            while (iss >> vertexSpec) {
                std::istringstream vss(vertexSpec);
                std::string part;
                std::vector<int> faceIndices;
                while (std::getline(vss, part, '/')) {
                    if (!part.empty()) {
                        faceIndices.push_back(std::stoi(part));
                    } else {
                        faceIndices.push_back(-1); // -1 indicates no data for this element
                    }
                }

                Vertex vertex;
                if (faceIndices.size() > 0) vertex.pos = positions[faceIndices[0] - 1];
                if (faceIndices.size() > 1 && faceIndices[1] != -1) vertex.uv = uvs[faceIndices[1] - 1];
                if (faceIndices.size() > 2 && faceIndices[2] != -1) vertex.normal = normals[faceIndices[2] - 1];

                faceVertices.push_back(vertex);
            }

            // Triangulate if it's a quad
            if (faceVertices.size() == 4) {
                // First triangle (v0, v1, v2)
                currentVertices.push_back(faceVertices[0]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                currentVertices.push_back(faceVertices[1]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                currentVertices.push_back(faceVertices[2]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));

                // Second triangle (v0, v2, v3)
                currentVertices.push_back(faceVertices[0]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                currentVertices.push_back(faceVertices[2]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                currentVertices.push_back(faceVertices[3]);
                currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
            } else {
                // For triangles, just add the vertices normally
                for (const Vertex& vertex : faceVertices) {
                    currentVertices.push_back(vertex);
                    currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                }
            }
        } else {
            std::cerr << "Unhandled field in .obj file: " << prefix << "\n";
            exit(1);
        }
    }

    if (currentVertices.size() > 0) {
        m_Meshes.push_back(new Mesh(currentVertices, currentIndices, currentMeshName, currentMaterialName));
    }
}

Model::~Model() {
    
    
}

void Model::Draw(Shader& shader, Matrix4 modelTransform, Matrix4 view, Matrix4 proj) {
    Vec3 sunDir{ 1.f, 1.f, 1.f };

    shader.Bind();

    shader.SetMat4("model", modelTransform);
    shader.SetMat4("view", view);
    shader.SetMat4("projection", proj);
    shader.SetVec3("sunDir", sunDir);
    shader.SetInt("hasDiffuseMap", 0);

    for (Mesh* mesh : m_Meshes) {

        Material* materialPtr = mesh->GetMaterialPtr();

        shader.SetVec3("diffuseColor", materialPtr->diffuseColor);
        if (materialPtr->diffuseMap.id) {
            shader.SetInt("hasDiffuseMap", 1);
            shader.SetInt("diffuseMap", 0);

            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, materialPtr->diffuseMap.id));
        }

        mesh->DrawCall();
    }
}
