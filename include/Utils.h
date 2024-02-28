#pragma once

#include "Maths.h"

std::string sameDirPath(const std::string& path, const std::string& otherFile);

struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec2 uv;
    Vec3 tangent;
    Vec3 bitangent;
};
struct Vec3;
void computeTangentBitangent(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vec3& tangent, Vec3& bitangent);
