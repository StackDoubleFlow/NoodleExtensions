#pragma once
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "Animation/Track.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"

namespace AnimationHelper {
struct ObjectOffset {
    // ObjectOffset() : positionOffset{UnityEngine::Vector3(0, 0, 0)}, scaleOffset{UnityEngine::Vector3(1, 1, 1)} {}
    std::optional<UnityEngine::Vector3> positionOffset;
    std::optional<UnityEngine::Quaternion> rotationOffset;
    std::optional<UnityEngine::Vector3> scaleOffset;
    std::optional<UnityEngine::Quaternion> localRotationOffset;
    std::optional<float> dissolve;
    std::optional<float> dissolveArrow;
    std::optional<float> cuttable;
};

std::optional<UnityEngine::Vector3> GetDefinitePositionOffset(const AnimationObjectData& animationData, Track *track, float time);
PointDefinition *TryGetPointData(BeatmapAssociatedData *beatmapAD, const rapidjson::Value& customData, std::string pointName);
ObjectOffset GetObjectOffset(const AnimationObjectData& customData, Track *track, float time);
void OnTrackCreated(Track *track);

} // end namespace AnimationHelper