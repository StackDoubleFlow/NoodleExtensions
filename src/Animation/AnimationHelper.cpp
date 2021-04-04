#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/AnimationHelper.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"

using namespace AnimationHelper;
using namespace GlobalNamespace;
using namespace UnityEngine;

// BeatmapObjectCallbackController.cpp
extern BeatmapObjectCallbackController *callbackController;

Vector3 vmult(Vector3 a, Vector3 b) {
    return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

PointDefinition *AnimationHelper::TryGetPointData(rapidjson::Value& customData, std::string pointName) {
    PointDefinition *pointData = nullptr;

    if (!customData.HasMember(pointName.c_str())) return pointData;
    rapidjson::Value& pointString = customData[pointName.c_str()];

    switch (pointString.GetType()) {
    case rapidjson::kNullType:
        return pointData;
    case rapidjson::kStringType:
        // TODO: Find global point data
        return pointData;
    default:
        pointData = new PointDefinition(pointString);
    }

    return pointData;
}

ObjectOffset AnimationHelper::GetObjectOffset(rapidjson::Value& customData, Track *track, float time) {
    ObjectOffset offset;

    // getBeatmapAD(((CustomJSONData::CustomBeatmapData*) callbackController->beatmapData)->customData);

    PointDefinition *position = TryGetPointData(customData, "_position");
    if (position) {
        offset.positionOffset = vmult(position->Interpolate(time), track ? track->properties.position.value.vector3 : Vector3());
    }

    PointDefinition *scale = TryGetPointData(customData, "_scale");
    if (scale) {
        offset.scaleOffset = vmult(scale->Interpolate(time), track ? track->properties.scale.value.vector3 : Vector3(1, 1, 1));
    }

    PointDefinition *cuttable = TryGetPointData(customData, "_cuttable");
    if (cuttable) {
        offset.cuttable = cuttable->InterpolateLinear(time) * ( track ? track->properties.cuttable.value.linear : 1);
    }

    return offset;
}

void AnimationHelper::OnTrackCreated(Track *track) {
    // track->properties.emplace("_position", Property(PropertyType::vector3, Vector3()));
    // track->properties.emplace("_rotation", Property(PropertyType::quaternion, Quaternion()));
    // track->properties.emplace("_scale", Property(PropertyType::vector3, Vector3(1, 1, 1)));
    // track->properties.emplace("_localRotation", Property(PropertyType::quaternion, Quaternion()));
    // track->properties.emplace("_dissolve", Property(PropertyType::linear, 1));
    // track->properties.emplace("_dissolveArrow", Property(PropertyType::linear, 1));
    // track->properties.emplace("_time", Property(PropertyType::linear, Quaternion()));
    // track->properties.emplace("_cuttable", Property(PropertyType::linear, 1));

    // track->pathProperties.emplace("_position", Property(PropertyType::vector3, Vector3()));
    // track->pathProperties.emplace("_rotation", Property(PropertyType::quaternion, Quaternion()));
    // track->pathProperties.emplace("_scale", Property(PropertyType::vector3, Vector3(1, 1, 1)));
    // track->pathProperties.emplace("_localRotation", Property(PropertyType::quaternion, Quaternion()));
    // track->pathProperties.emplace("_definitePosition", Property(PropertyType::vector3, Vector3()));
    // track->pathProperties.emplace("_dissolve", Property(PropertyType::linear, 1));
    // track->pathProperties.emplace("_dissolveArrow", Property(PropertyType::linear, 1));
    // track->pathProperties.emplace("_cuttable", Property(PropertyType::linear, 1));
}