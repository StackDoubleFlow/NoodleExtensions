#pragma once

#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"

namespace NEVector {

struct Vector2 : public UnityEngine::Vector2 {
    Vector2(float x = 0, float y = 0) : UnityEngine::Vector2(x, y) {}
    Vector2(const UnityEngine::Vector2 &other) : UnityEngine::Vector2(other) {}
};

struct Vector3 : public UnityEngine::Vector3 {
    Vector3(float x = 0, float y = 0, float z = 0)
        : UnityEngine::Vector3(x, y, z) {}
    Vector3(const UnityEngine::Vector3 &other) : UnityEngine::Vector3(other) {}

    static Vector3 get_zero() { return Vector3(0, 0, 0); }

    Vector3 operator+(const Vector3 &rhs) {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3 &rhs) {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3 operator*(const Vector3 &rhs) {
        return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    Vector3 operator*(float rhs) { return Vector3(x * rhs, y * rhs, z * rhs); }

    static Vector3 LerpUnclamped(const Vector3 &a, const Vector3 &b, float t) {
        return Vector3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                       a.z + (b.z - a.z) * t);
    }
};

struct Vector4 : public UnityEngine::Vector4 {
    Vector4(float x = 0, float y = 0, float z = 0, float w = 0)
        : UnityEngine::Vector4(x, y, z, w) {}
    Vector4(const UnityEngine::Vector4 &other) : UnityEngine::Vector4(other) {}

    operator Vector3() const { return Vector3(x, y, z); }
};

struct Quaternion : public UnityEngine::Quaternion {
    Quaternion(float x = 0, float y = 0, float z = 0, float w = 1)
        : UnityEngine::Quaternion(x, y, z, w) {}
    Quaternion(const UnityEngine::Quaternion &other)
        : UnityEngine::Quaternion(other) {}

    Quaternion operator*(const Quaternion &rhs) {
        return Quaternion(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
                          w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
                          w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
                          w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }
};

struct Vector5 {
    float x, y, z, w, v;

    Vector5(float x = 0, float y = 0, float z = 0, float w = 0, float v = 0)
        : x{x}, y{y}, z{z}, w{w}, v{v} {};

    operator Vector4() const { return Vector4(x, y, z, w); }
};

} // end namespace NEVector