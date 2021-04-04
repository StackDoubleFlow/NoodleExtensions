#pragma once
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Quaternion.hpp"

class PointDefinition;

class PointDefinitionInterpolation {
public:
    UnityEngine::Vector3 Interpolate(float time);
    UnityEngine::Quaternion InterpolateQuaternion(float time);
    float InterpolateLinear(float time);
    UnityEngine::Vector4 InterpolateVector4(float time);

    float time;
private:
    PointDefinition *basePointData;
    PointDefinition *previousPointData;
};