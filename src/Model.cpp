#include <fstream>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <map>
#include <limits>
#include <filesystem>
namespace fs = std::filesystem;

#include "Model.h"
#include "Utils.h"
#include "Scene.h"


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

    GL_CALL(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent)));
    GL_CALL(glEnableVertexAttribArray(3));

    GL_CALL(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent)));
    GL_CALL(glEnableVertexAttribArray(4));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    std::cout << "Created mesh object '" << m_Name << "' with " << m_Vertices.size() << " vertices and " << m_Indicies.size() << " indices.\n";
}
Mesh::~Mesh() {
    GL_CALL(glDeleteVertexArrays(1, &m_VAO));
    GL_CALL(glDeleteBuffers(1, &m_VBO));
    GL_CALL(glDeleteBuffers(1, &m_EBO));
}

void Mesh::DrawCall(Shader& shader) {

    TextureBindContext::ApplyAll();

    checkProgram(shader.GetProgramID());

    GL_CALL(glBindVertexArray(m_VAO));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
    GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)m_Indicies.size(), GL_UNSIGNED_INT, 0));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
}


Model::Model(const std::string& objPath) {
    std::ifstream file(objPath);
    assert(file.is_open() && "Failed to open file");

    std::cout << "Loading model from '" << objPath << "'...";

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;

    std::vector<Vertex> currentVertices; // Vertices for current mesh
    std::vector<unsigned int> currentIndices;
    std::string currentMeshName = "UNNAMED";
    std::string currentMaterialName = "UNNAMED";

    // Parsing wavefront .obj model according to:
    // https://en.wikipedia.org/wiki/Wavefront_.obj_file

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
            iss >> value; // Ignore
        } else if (prefix == "o") {
            if (currentVertices.size() > 0) {
                m_Meshes.push_back(new Mesh(currentVertices, currentIndices, currentMeshName, currentMaterialName));

                currentVertices.clear();
                currentIndices.clear();
            }

            iss >> currentMeshName;

        } else if (prefix == "mtllib") {
            std::string path;
            iss >> path;
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
            uv.y = 1.f - uv.y;
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

            Vec3 tangent, bitangent;

            // Triangulate if it's a quad
            if (faceVertices.size() == 4) {
                // Calculate tangent and bitangent for the first triangle
                computeTangentBitangent(faceVertices[0], faceVertices[1], faceVertices[2], tangent, bitangent);

                // Assign tangent and bitangent to vertices
                for (int i = 0; i < 3; ++i) {
                    faceVertices[i].tangent = tangent;
                    faceVertices[i].bitangent = bitangent;
                }

                // First triangle (v0, v1, v2)
                currentVertices.insert(currentVertices.end(), faceVertices.begin(), faceVertices.begin() + 3);
                for (int i = 0; i < 3; ++i) {
                    currentIndices.push_back(static_cast<int>(currentVertices.size() - 3 + i));
                }

                // Calculate tangent and bitangent for the second triangle
                computeTangentBitangent(faceVertices[0], faceVertices[2], faceVertices[3], tangent, bitangent);

                // Assign tangent and bitangent to vertices (v0, v2, v3)
                faceVertices[0].tangent = faceVertices[2].tangent = faceVertices[3].tangent = tangent;
                faceVertices[0].bitangent = faceVertices[2].bitangent = faceVertices[3].bitangent = bitangent;

                // Second triangle (v0, v2, v3)
                currentVertices.push_back(faceVertices[0]);
                currentVertices.push_back(faceVertices[2]);
                currentVertices.push_back(faceVertices[3]);
                for (int i = 0; i < 3; ++i) {
                    currentIndices.push_back(static_cast<int>(currentVertices.size() - 3 + i));
                }
            } else {
                // For triangles, just add the vertices normally
                computeTangentBitangent(faceVertices[0], faceVertices[1], faceVertices[2], tangent, bitangent);

                // Assign tangent and bitangent to vertices
                for (Vertex& vertex : faceVertices) {
                    vertex.tangent = tangent;
                    vertex.bitangent = bitangent;
                    currentVertices.push_back(vertex);
                    currentIndices.push_back(static_cast<int>(currentVertices.size() - 1));
                }
            }
        } else if (prefix == "r" || prefix == "g" || prefix == "b"
            || prefix == "m" || prefix == "l" || prefix == "z") { // Scalar or Bump texture channel (?)
            std::string dummy;
            iss >> dummy; // Ignore
        } else if(prefix != "") {
            std::cerr << "Unhandled field in .obj file: " << prefix << "\n";
            CRASH();
        }
    }

    if (currentVertices.size() > 0) {
        m_Meshes.push_back(new Mesh(currentVertices, currentIndices, currentMeshName, currentMaterialName));
    }

    std::cout << "Model loading OK!\n";
}

Model::~Model() {
    for (auto mesh : m_Meshes) {
        delete mesh;
    }
}

void Model::Draw(Scene& scene, Shader& shader, int fromActiveTexture) {
    Vec3 sunDir{ 1.f, 1.f, 1.f };

    shader.SetMat4("model", m_Transform);

    for (Mesh* mesh : m_Meshes) {

        Material* materialPtr = mesh->GetMaterialPtr();

        setMaterialInShader(*materialPtr, shader, fromActiveTexture);

        mesh->DrawCall(shader);
    }
}
