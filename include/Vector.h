#pragma once

#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Quaternion.hpp"

struct Vector2 : public UnityEngine::Vector2 {
    Vector2(float x, float y) : UnityEngine::Vector2(x, y) {}
    Vector2(const UnityEngine::Vector2& other) : UnityEngine::Vector2(other) {}
};

struct Vector3 : public UnityEngine::Vector3 {
    Vector3(float x, float y, float z) : UnityEngine::Vector3(x, y, z) {}
    Vector3(const UnityEngine::Vector3& other) : UnityEngine::Vector3(other) {}
};

struct Vector4 : public UnityEngine::Vector4 {
    Vector4(float x, float y, float z, float w) : UnityEngine::Vector4(x, y, z, w) {}
    Vector4(const UnityEngine::Vector4& other) : UnityEngine::Vector4(other) {}

    operator Vector3() const {
        return Vector3(x, y, z);
    }
};

struct Quaternion : public UnityEngine::Quaternion {
    Quaternion(float x, float y, float z, float w) : UnityEngine::Quaternion(x, y, z, w) {}
    Quaternion(const UnityEngine::Quaternion& other) : UnityEngine::Quaternion(other) {}
};
