#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "Utils.h"
#include "../include/Maths.h"

std::string sameDirPath(const std::string& path, const std::string& otherFile) {
    std::string dir = fs::path(path).parent_path().string();
    return dir + "/" + otherFile;
}

void computeTangentBitangent(const Vertex& v0, const Vertex& v1, const Vertex& v2, Vec3& tangent, Vec3& bitangent) {
    Vec3 edge1 = v1.pos.Subtract(v0.pos);
    Vec3 edge2 = v2.pos.Subtract(v0.pos);
    Vec2 deltaUV1 = v1.uv.Subtract(v0.uv);
    Vec2 deltaUV2 = v2.uv.Subtract(v0.uv);

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = tangent.Normalized();

    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = bitangent.Normalized();
}