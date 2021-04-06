#pragma once
#include "Animation/Easings.h"
#include "UnityEngine/Vector2.hpp"
#include "UnityEngine/Vector4.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "Animation/Track.h"

struct Vector5 {
    float x, y, z, w, v;

    Vector5(float x, float y, float z, float w, float v) : x{x}, y{y}, z{z}, w{w}, v{v} {};
};

struct PointData {
    UnityEngine::Vector2 linearPoint = UnityEngine::Vector2(0, 0);
    UnityEngine::Vector4 point = UnityEngine::Vector4(0, 0, 0, 0);
    Vector5 vector4Point = Vector5(0, 0, 0, 0, 0);
    Functions easing = Functions::easeLinear;
    bool smooth = false;

    PointData(UnityEngine::Vector4 point, Functions easing = Functions::easeLinear, bool smooth = false) : point{point}, easing{easing}, smooth{smooth} {};
    PointData(UnityEngine::Vector2 point, Functions easing = Functions::easeLinear) : linearPoint{point}, easing{easing} {};
    PointData(Vector5 point, Functions easing = Functions::easeLinear) : vector4Point{point}, easing{easing} {};
};

class PointDefinition {
public:
    PointDefinition(const rapidjson::Value& value);
    UnityEngine::Vector3 Interpolate(float time);
    UnityEngine::Quaternion InterpolateQuaternion(float time);
    float InterpolateLinear(float time);
    UnityEngine::Vector4 InterpolateVector4(float time);
private:
    void SearchIndex(float time, PropertyType propertyType, int& l, int& r);
    std::vector<PointData> points;
};

class PointDefinitionManager {
public:
    std::unordered_map<std::string, PointDefinition> pointData;

    void AddPoint(std::string pointDataName, PointDefinition pointData);
};