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

    Vector3 operator+(const Vector3 &rhs) const {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3 &rhs) const {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3 operator*(const Vector3 &rhs) const {
        return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    Vector3 operator*(const float &rhs) const {
        return Vector3(x * rhs, y * rhs, z * rhs);
    }

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

    static Quaternion get_identity() { return Quaternion(0, 0, 0, 1); }

    // static float Dot(const Quaternion &a, const Quaternion &b) {
    //     return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    // }

    // static Quaternion Inverse(const Quaternion &a) {
    //     Quaternion conj = {-a.x, -a.y, -a.z, a.w};
    //     float norm2 = Quaternion::Dot(a, a);
    //     return Quaternion(conj.x / norm2, conj.y / norm2, conj.z / norm2,
    //                       conj.w / norm2);
    // }

    // static Quaternion Euler(const Vector3 &euler) {
    //     Vector3 rad = euler * 0.017453292f;
    //     float yaw = rad.z;
    //     float pitch = rad.y;
    //     float roll = rad.x;
    //     float cy = std::cosf(yaw * 0.5);
    //     float sy = std::sinf(yaw * 0.5);
    //     float cp = std::cosf(pitch * 0.5);
    //     float sp = std::sinf(pitch * 0.5);
    //     float cr = std::cosf(roll * 0.5);
    //     float sr = std::sinf(roll * 0.5);

    //     Quaternion q;
    //     q.w = cr * cp * cy + sr * sp * sy;
    //     q.x = sr * cp * cy - cr * sp * sy;
    //     q.y = cr * sp * cy + sr * cp * sy;
    //     q.z = cr * cp * sy - sr * sp * cy;

    //     return q;
    // }

    // static Quaternion SlerpUnclamped(const Quaternion &q1, const Quaternion &q2, double lambda) {
    //     Quaternion qr;
    //     float dotproduct = Quaternion::Dot(q1, q2);
    //     float theta, st, sut, sout, coeff1, coeff2;

    //     // algorithm adapted from Shoemake's paper
    //     lambda = lambda / 2.0;

    //     theta = acosf(dotproduct);
    //     if (theta < 0.0)
    //         theta = -theta;

    //     st = sinf(theta);
    //     sut = sinf(lambda * theta);
    //     sout = sinf((1 - lambda) * theta);
    //     coeff1 = sout / st;
    //     coeff2 = sut / st;

    //     qr.x = coeff1 * q1.x + coeff2 * q2.x;
    //     qr.y = coeff1 * q1.y + coeff2 * q2.y;
    //     qr.z = coeff1 * q1.z + coeff2 * q2.z;
    //     qr.w = coeff1 * q1.w + coeff2 * q2.w;

    //     // qr.Normalize();
    //     return qr;
    // }

    // static Quaternion Euler(float x, float y, float z) {
    //     return Quaternion::Euler(Vector3(x, y, z));
    // }
};

struct Vector5 {
    float x, y, z, w, v;

    Vector5(float x = 0, float y = 0, float z = 0, float w = 0, float v = 0)
        : x{x}, y{y}, z{z}, w{w}, v{v} {};

    operator Vector4() const { return Vector4(x, y, z, w); }
};

} // end namespace NEVector