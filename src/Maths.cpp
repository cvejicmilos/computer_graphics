#include "../include/Maths.h"

Matrix4::Matrix4() {
    for (int i = 0; i < 16; ++i) data[i] = 0.0f;
    data[0] = data[5] = data[10] = data[15] = 1.0f; 
}

Matrix4& Matrix4::SetTranslation(const Vec3& v) {
    // Set position coords of matrix
    data[12] = v.x;
    data[13] = v.y;
    data[14] = v.z;

    return *this;
}

Matrix4& Matrix4::Translate(const Vec3& v) {
    // Add to position coords of matrix
    data[12] += v.x;
    data[13] += v.y;
    data[14] += v.z;

    return *this;
}

Matrix4& Matrix4::SetScale(const Vec3& v) {
    // Set scale parts in matrix
    data[0] = v.x;
    data[5] = v.y;
    data[10] = v.z;

    return *this;
}

Matrix4& Matrix4::Scale(const Vec3& v) {
    // Add (multiply) to scale parts in matrix
    data[0] *= v.x;
    data[5] *= v.y;
    data[10] *= v.z;

    return *this;
}


Matrix4& Matrix4::SetRotation(float angle, const Vec3& axis) {

    float c = cos(angle);
    float s = sin(angle);
    float omc = 1.0f - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    data[0] = x * x * omc + c;
    data[1] = y * x * omc + z * s;
    data[2] = x * z * omc - y * s;

    data[4] = x * y * omc - z * s;
    data[5] = y * y * omc + c;
    data[6] = y * z * omc + x * s;

    data[8] = x * z * omc + y * s;
    data[9] = y * z * omc - x * s;
    data[10] = z * z * omc + c;

    return *this;
}

Matrix4& Matrix4::Rotate(float angle, const Vec3& axis) {
    Matrix4 rotationMatrix;
    rotationMatrix.SetRotation(angle, axis);

    *this = rotationMatrix * (*this);

    return *this;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            result.data[row * 4 + col] = 0.0f;
            for (int i = 0; i < 4; ++i) {
                result.data[row * 4 + col] += data[row * 4 + i] * other.data[i * 4 + col];
            }
        }
    }
    return result;
}

Vec4 Matrix4::Multiply(const Vec4& other) const {
    return Vec4{
        data[0] * other.x + data[4] * other.y + data[8] * other.z + data[12] * other.w,
        data[1] * other.x + data[5] * other.y + data[9] * other.z + data[13] * other.w,
        data[2] * other.x + data[6] * other.y + data[10] * other.z + data[14] * other.w,
        data[3] * other.x + data[7] * other.y + data[11] * other.z + data[15] * other.w
    };
}

Vec3 Matrix4::Multiply(const Vec3& other) const {
    Vec4 temp = Multiply(Vec4{other.x, other.y, other.z, 1.0f});
    return Vec3{temp.x, temp.y, temp.z};
}

Matrix4 Matrix4::Identity() {
    Matrix4 mat = {};
    mat.data[0] = mat.data[5] = mat.data[10] = mat.data[15] = 1.0f;
    return mat;
}

Matrix4 Matrix4::CreateTranslation(const Vec3& v) {
    Matrix4 mat = Identity();
    mat.data[12] = v.x;
    mat.data[13] = v.y;
    mat.data[14] = v.z;
    return mat;
}

Matrix4 Matrix4::Perspective(float fov, float aspect, float near, float far) {
    Matrix4 mat;
    float tanHalfFovy = tan(fov / 2.0f);
    mat.data[0] = 1.0f / (aspect * tanHalfFovy);
    mat.data[5] = 1.0f / tanHalfFovy;
    mat.data[10] = -(far + near) / (far - near);
    mat.data[11] = -1.0f;
    mat.data[14] = -(2.0f * far * near) / (far - near);
    mat.data[15] = 0.0f;
    return mat;
}

Matrix4& Matrix4::Invert() {
    float inv[16], det;
    int i;

    inv[0] = data[5] * data[10] * data[15] - 
             data[5] * data[11] * data[14] - 
             data[9] * data[6] * data[15] + 
             data[9] * data[7] * data[14] +
             data[13] * data[6] * data[11] - 
             data[13] * data[7] * data[10];

    inv[4] = -data[4] * data[10] * data[15] + 
             data[4] * data[11] * data[14] + 
             data[8] * data[6] * data[15] - 
             data[8] * data[7] * data[14] - 
             data[12] * data[6] * data[11] + 
             data[12] * data[7] * data[10];

    inv[8] = data[4] * data[9] * data[15] - 
             data[4] * data[11] * data[13] - 
             data[8] * data[5] * data[15] + 
             data[8] * data[7] * data[13] + 
             data[12] * data[5] * data[11] - 
             data[12] * data[7] * data[9];

    inv[12] = -data[4] * data[9] * data[14] + 
               data[4] * data[10] * data[13] +
               data[8] * data[5] * data[14] - 
               data[8] * data[6] * data[13] - 
               data[12] * data[5] * data[10] + 
               data[12] * data[6] * data[9];

    inv[1] = -data[1] * data[10] * data[15] + 
              data[1] * data[11] * data[14] + 
              data[9] * data[2] * data[15] - 
              data[9] * data[3] * data[14] - 
              data[13] * data[2] * data[11] + 
              data[13] * data[3] * data[10];

    inv[5] = data[0] * data[10] * data[15] - 
             data[0] * data[11] * data[14] - 
             data[8] * data[2] * data[15] + 
             data[8] * data[3] * data[14] + 
             data[12] * data[2] * data[11] - 
             data[12] * data[3] * data[10];

    inv[9] = -data[0] * data[9] * data[15] + 
              data[0] * data[11] * data[13] + 
              data[8] * data[1] * data[15] - 
              data[8] * data[3] * data[13] - 
              data[12] * data[1] * data[11] + 
              data[12] * data[3] * data[9];

    inv[13] = data[0] * data[9] * data[14] - 
              data[0] * data[10] * data[13] - 
              data[8] * data[1] * data[14] + 
              data[8] * data[2] * data[13] + 
              data[12] * data[1] * data[10] - 
              data[12] * data[2] * data[9];

    inv[2] = data[1] * data[6] * data[15] - 
             data[1] * data[7] * data[14] - 
             data[5] * data[2] * data[15] + 
             data[5] * data[3] * data[14] + 
             data[13] * data[2] * data[7] - 
             data[13] * data[3] * data[6];

    inv[6] = -data[0] * data[6] * data[15] + 
              data[0] * data[7] * data[14] + 
              data[4] * data[2] * data[15] - 
              data[4] * data[3] * data[14] - 
              data[12] * data[2] * data[7] + 
              data[12] * data[3] * data[6];

    inv[10] = data[0] * data[5] * data[15] - 
              data[0] * data[7] * data[13] - 
              data[4] * data[1] * data[15] + 
              data[4] * data[3] * data[13] + 
              data[12] * data[1] * data[7] - 
              data[12] * data[3] * data[5];

    inv[14] = -data[0] * data[5] * data[14] + 
               data[0] * data[6] * data[13] + 
               data[4] * data[1] * data[14] - 
               data[4] * data[2] * data[13] - 
               data[12] * data[1] * data[6] + 
               data[12] * data[2] * data[5];

    inv[3] = -data[1] * data[6] * data[11] + 
              data[1] * data[7] * data[10] + 
              data[5] * data[2] * data[11] - 
              data[5] * data[3] * data[10] - 
              data[9] * data[2] * data[7] + 
              data[9] * data[3] * data[6];

    inv[7] = data[0] * data[6] * data[11] - 
             data[0] * data[7] * data[10] - 
             data[4] * data[2] * data[11] + 
             data[4] * data[3] * data[10] + 
             data[8] * data[2] * data[7] - 
             data[8] * data[3] * data[6];

    inv[11] = -data[0] * data[5] * data[11] + 
               data[0] * data[7] * data[9] + 
               data[4] * data[1] * data[11] - 
               data[4] * data[3] * data[9] - 
               data[8] * data[1] * data[7] + 
               data[8] * data[3] * data[5];

    inv[15] = data[0] * data[5] * data[10] - 
              data[0] * data[6] * data[9] - 
              data[4] * data[1] * data[10] + 
              data[4] * data[2] * data[9] + 
              data[8] * data[1] * data[6] - 
              data[8] * data[2] * data[5];

    det = data[0] * inv[0] + data[1] * inv[4] + data[2] * inv[8] + data[3] * inv[12];

    if (det == 0) {
        // Matrix is not invertible. In this case, we'll just return the unchanged matrix.
        return *this;
    }

    det = 1.0f / det;

    for (i = 0; i < 16; i++) {
        data[i] = inv[i] * det;
    }

    return *this;
}

Vec3 Matrix4::TransformDirection(const Vec3& v) const {
    return Vec3{
        data[0] * v.x + data[4] * v.y + data[8] * v.z,
        data[1] * v.x + data[5] * v.y + data[9] * v.z,
        data[2] * v.x + data[6] * v.y + data[10] * v.z
    };
}

Vec3 Matrix4::GetTranslation() const {
    Vec3 v;
    v.x = data[12];
    v.y = data[13];
    v.z = data[14];
    
    return v;
}