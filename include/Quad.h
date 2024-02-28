#pragma once

#include <vector>

#include "GLutils.h"
#include "Maths.h"

class Shader;

class GrassQuad {
private:
    Matrix4 m_Transform;
    Vec2 m_Size;
public:
    GrassQuad(Vec3 position, Vec2 size, Vec3 rotation);

    const Matrix4& GetTransform() const;

    Vec2 GetSize() const { return m_Size; }
};

void batchDrawGrass(std::vector<GrassQuad>& quads, Shader& shader, Texture texture, Texture normalMap);