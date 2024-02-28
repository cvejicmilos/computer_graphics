#pragma once

#include <cmath>

#define PI32 (3.14159265359f)

// Simple math types for 3D graphics (Vec3 & Matrix4)

struct Vec2 {
    float x, y;

    Vec2 Subtract(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }

    float Length() const {
        return sqrt((x * x) + (y * y));
    }

    Vec2 Normalized() const {
        float len = Length();
        return Vec2{x / len, y / len};
    }
};
struct Vec3 {
    float x, y, z;

    Vec3 Subtract(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    float Length() const {
        return sqrt((x * x) + (y * y) + (z * z));
    }

    Vec3 Normalized() const {
        float len = Length();
        return Vec3{x / len, y / len, z / len};
    }
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

    Vec4 Multiply(const Vec4& other) const;
    Vec3 Multiply(const Vec3& other) const;

    Matrix4& Invert();
    Vec3 TransformDirection(const Vec3& v) const;

    Vec3 GetTranslation() const;

    // Helper functions to create matrix
    static Matrix4 Identity();
    static Matrix4 CreateTranslation(const Vec3& v);
    static Matrix4 Perspective(float fov, float aspect, float near, float far);
};