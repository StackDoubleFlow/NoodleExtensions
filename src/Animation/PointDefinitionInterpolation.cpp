#include "Animation/PointDefinitionInterpolation.h"
#include "Animation/PointDefinition.h"

using namespace UnityEngine;

UnityEngine::Vector3 PointDefinitionInterpolation::Interpolate(float time) {
    return Vector3::LerpUnclamped(previousPointData->Interpolate(time), basePointData->Interpolate(time), this->time);
}

UnityEngine::Quaternion PointDefinitionInterpolation::InterpolateQuaternion(float time) {
    return Quaternion::SlerpUnclamped(previousPointData->InterpolateQuaternion(time), basePointData->InterpolateQuaternion(time), this->time);
}

float PointDefinitionInterpolation::InterpolateLinear(float time) {
    return std::lerp(previousPointData->InterpolateLinear(time), basePointData->InterpolateLinear(time), this->time);
}

UnityEngine::Vector4 PointDefinitionInterpolation::InterpolateVector4(float time) {
    Vector4 a = previousPointData->InterpolateVector4(time);
    Vector4 b = basePointData->InterpolateVector4(time);
    float t = this->time;
    return Vector4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}