#pragma once

#include <cmath>

#define PI32 (3.14159265359f)

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
        if (len == 0) return Vec2 { 0, 0 };
        return Vec2{x / len, y / len};
    }
};
struct Vec3 {
    float x, y, z;

    Vec3() {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z){}

    Vec3 Subtract(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z };
    }
    Vec3 Add(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z };
    }

    Vec3 Multiply(float other) const {
        return {x * other, y * other, z * other };
    }

    float Length() const {
        return sqrt((x * x) + (y * y) + (z * z));
    }

    Vec3 Invert() const {
        return { -x, -y, -z };
    }

    Vec3 Normalized() const {
        float len = Length();
        if (len == 0) return Vec3(0, 0, 0);
        return Vec3{x / len, y / len, z / len};
    }

    static Vec3 Cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static Vec3 Mix(const Vec3& start, const Vec3& end, float factor) {
        return Vec3(
            start.x + (end.x - start.x) * factor,
            start.y + (end.y - start.y) * factor,
            start.z + (end.z - start.z) * factor
        );
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
    static Matrix4 CreatePerspective(float fov, float aspect, float near, float far);
    static Matrix4 CreateLookAt(const Vec3& cameraPos, const Vec3& target, const Vec3& upWorld);
    static Matrix4 CreateOrtho(float left, float right, float bottom, float top, float near, float far);
};