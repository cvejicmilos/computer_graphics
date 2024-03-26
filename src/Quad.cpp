
#include "Quad.h"

#include "Material.h"
#include "Shader.h"
#include "Utils.h"

GrassQuad::GrassQuad(Vec3 position, Vec2 size, Vec3 rotation) {
    m_Transform = Matrix4::CreateTranslation(position)
                .Rotate(rotation.x, { 1, 0, 0 })
                .Rotate(rotation.y, { 0, 1, 0 })
                .Rotate(rotation.z, { 0, 0, 1 });

    if (!MaterialLibrary::Get().ExistsMaterial("basicQuad")) {
        Material quadMaterial;

        quadMaterial.specularExponent = 8;

        MaterialLibrary::Get().AddMaterial("basicQuad", quadMaterial);
    }

    m_Size = size;
}

const Matrix4& GrassQuad::GetTransform() const  {
    return m_Transform;
}

std::vector<Vertex> g_AllVertices;
GLuint g_QuadsVAO = 0, g_QuadsVBO = 0, g_QuadsEBO = 0;
size_t currentBufferSize = 0;
const size_t MAX_QUADS = 30000; // Maximum number of quads that can be batched
const size_t QUAD_VERTEX_COUNT = 4;
const size_t QUAD_INDEX_COUNT = 6;
const size_t MAX_INDICES = MAX_QUADS * QUAD_INDEX_COUNT;

void batchDrawGrass(std::vector<GrassQuad>& quads, Shader& shader, Texture texture, Texture normalMap, int nextActiveTexture) {
    g_AllVertices.clear();

    // Initialize buffers if they are zero
    if (g_QuadsVAO == 0) {
        glGenVertexArrays(1, &g_QuadsVAO);
        glGenBuffers(1, &g_QuadsVBO);
        glGenBuffers(1, &g_QuadsEBO);

        glBindVertexArray(g_QuadsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, g_QuadsVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_QuadsEBO);
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
    }

    GL_CALL(glBindVertexArray(g_QuadsVAO));

    // Resize buffers if necessary
    size_t requiredBufferSize = quads.size() * sizeof(Vertex) * QUAD_VERTEX_COUNT;
    if (requiredBufferSize > currentBufferSize) {
        size_t newBufferSize = std::max(requiredBufferSize, (size_t)(currentBufferSize * 1.5));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_QuadsVBO));
        GL_CALL(glBufferData(GL_ARRAY_BUFFER, newBufferSize, NULL, GL_DYNAMIC_DRAW)); // Resizing buffer
        currentBufferSize = newBufferSize;
    }

    // Update VBO with quad data
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, g_QuadsVBO));
    for (size_t i = 0; i < quads.size(); ++i) {
        GrassQuad& quad = quads[i];
        const Matrix4& transform = quad.GetTransform();
        Vec3 size = { quad.GetSize().x, quad.GetSize().y, 0 };
        Vec3 normTransformed = transform.TransformDirection(Vec3{ 0, 0, -1 });

        // Add vertices of the quad
        for (int j = 0; j < 4; ++j) {
            Vertex vert;
            vert.pos = transform.Multiply(Vec3{ (j == 0 || j == 1) ? -size.x/2.f : size.x/2.f, 
                                                (j == 0 || j == 3) ? -size.y/2.f : size.y/2.f, 0 });
            vert.uv = Vec2{(float)(j == 2 || j == 3), (float)(j == 0 || j == 3)};
            vert.normal = normTransformed;
            g_AllVertices.push_back(vert);
        }

        // Compute Tangent and Bitangent for the last quad added
        Vec3 tangent, bitangent;
        computeTangentBitangent(g_AllVertices[g_AllVertices.size() - 4],
                                g_AllVertices[g_AllVertices.size() - 3],
                                g_AllVertices[g_AllVertices.size() - 2], 
                                tangent, bitangent);

        // Update the tangent and bitangent for the last 4 vertices (the quad)
        for (size_t j = g_AllVertices.size() - 4; j < g_AllVertices.size(); ++j) {
            g_AllVertices[j].tangent = tangent;
            g_AllVertices[j].bitangent = bitangent;
        }
    }
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * g_AllVertices.size(), g_AllVertices.data()));

    // Set up indices for EBO
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_QuadsEBO));
    GLuint indices[MAX_INDICES];
    for (int i = 0, offset = 0; i < quads.size(); ++i, offset += (int)QUAD_VERTEX_COUNT) {
        indices[i * QUAD_INDEX_COUNT] = offset + 0;
        indices[i * QUAD_INDEX_COUNT + 1] = offset + 1;
        indices[i * QUAD_INDEX_COUNT + 2] = offset + 2;
        indices[i * QUAD_INDEX_COUNT + 3] = offset + 2;
        indices[i * QUAD_INDEX_COUNT + 4] = offset + 3;
        indices[i * QUAD_INDEX_COUNT + 5] = offset + 0;
    }
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, quads.size() * QUAD_INDEX_COUNT * sizeof(GLuint), indices, GL_STATIC_DRAW));


    Material* materialPtr = MaterialLibrary::Get().GetMaterial("basicQuad");
    materialPtr->ambientMap = texture;
    materialPtr->diffuseMap = texture;
    materialPtr->normalMap = normalMap;

    setMaterialInShader(*materialPtr, shader, nextActiveTexture);

    TextureBindContext::ApplyAll();
    checkProgram(shader.GetProgramID());

    // Draw for each side
    shader.SetMat4("model", Matrix4::Identity());
    GL_CALL(glDrawElements(GL_TRIANGLES, (GLsizei)quads.size() * QUAD_INDEX_COUNT, GL_UNSIGNED_INT, 0));

    // Unbind VAO
    glBindVertexArray(0);
}