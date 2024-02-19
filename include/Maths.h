#pragma once

#include <cmath>

// Simple math types for 3D graphics (Vec3 & Matrix4)

struct Vec2 {
    float x, y;
};
struct Vec3 {
    float x, y, z;
};
struct Vec4 {
    float x, y, z, w;
};

struct Matrix4 {
    float data[16];

    Matrix4();

    // Set translation (position)
    Matrix4& SetTranslation(const Vec3& v);
    // Accumulate translation (position)
    Matrix4& Translate(const Vec3& v);

    // Set scale
    Matrix4& SetScale(const Vec3& v);
    // Accumulate scale
    Matrix4& Scale(const Vec3& v);

    // Set rotation
    Matrix4& SetRotation(float angle, const Vec3& axis);
    // Accumulate rotation
    Matrix4& Rotate(float angle, const Vec3& axis);

    Matrix4 operator*(const Matrix4& other) const;

    Matrix4& Invert();
    Vec3 TransformDirection(const Vec3& v) const;

    // Helper functions to create matrix
    static Matrix4 Identity();
    static Matrix4 CreateTranslation(const Vec3& v);
    static Matrix4 Perspective(float fov, float aspect, float near, float far);
};