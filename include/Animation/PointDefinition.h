#pragma once
#include "Animation/Easings.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "Animation/Track.h"
#include "Vector.h"

struct PointData {
    NEVector::Vector2 linearPoint = NEVector::Vector2(0, 0);
    NEVector::Vector4 point = NEVector::Vector4(0, 0, 0, 0);
    NEVector::Vector5 vector4Point = NEVector::Vector5(0, 0, 0, 0, 0);
    Functions easing = Functions::easeLinear;
    bool smooth = false;

    PointData(NEVector::Vector4 point, Functions easing = Functions::easeLinear, bool smooth = false) : point{point}, easing{easing}, smooth{smooth} {};
    PointData(NEVector::Vector2 point, Functions easing = Functions::easeLinear) : linearPoint{point}, easing{easing} {};
    PointData(NEVector::Vector5 point, Functions easing = Functions::easeLinear) : vector4Point{point}, easing{easing} {};
};

class PointDefinition {
public:
    PointDefinition(const rapidjson::Value& value);
    NEVector::Vector3 Interpolate(float time);
    NEVector::Quaternion InterpolateQuaternion(float time);
    float InterpolateLinear(float time);
    NEVector::Vector4 InterpolateVector4(float time);
private:
    void SearchIndex(float time, PropertyType propertyType, int& l, int& r);
    std::vector<PointData> points;
};

class PointDefinitionManager {
public:
    std::unordered_map<std::string, PointDefinition> pointData;

    void AddPoint(std::string pointDataName, PointDefinition pointData);
};