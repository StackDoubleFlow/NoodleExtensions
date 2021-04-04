#pragma once
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "Animation/Track.h"
#include "Animation/PointDefinition.h"

namespace AnimationHelper {
struct ObjectOffset {
    ObjectOffset() : positionOffset{UnityEngine::Vector3(0, 0, 0)}, scaleOffset{UnityEngine::Vector3(1, 1, 1)} {}
    UnityEngine::Vector3 positionOffset;
    UnityEngine::Quaternion rotationOffset;
    UnityEngine::Vector3 scaleOffset;
    UnityEngine::Quaternion localRotationOffset;
    float dissolve;
    float dissolveArrow;
    float cuttable;
};

PointDefinition *TryGetPointData(rapidjson::Value& customData, std::string pointName);
ObjectOffset GetObjectOffset(rapidjson::Value& customData, Track *track, float time);
void OnTrackCreated(Track *track);

} // end namespace AnimationHelper