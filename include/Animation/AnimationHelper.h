#pragma once
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "Animation/Track.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"
#include "Vector.h"

namespace AnimationHelper {
struct ObjectOffset {
    // ObjectOffset() : positionOffset{UnityEngine::Vector3(0, 0, 0)}, scaleOffset{UnityEngine::Vector3(1, 1, 1)} {}
    std::optional<NEVector::Vector3> positionOffset;
    std::optional<NEVector::Quaternion> rotationOffset;
    std::optional<NEVector::Vector3> scaleOffset;
    std::optional<NEVector::Quaternion> localRotationOffset;
    std::optional<float> dissolve;
    std::optional<float> dissolveArrow;
    std::optional<float> cuttable;
};

std::optional<NEVector::Vector3> GetDefinitePositionOffset(const AnimationObjectData& animationData, Track *track, float time);
PointDefinition *TryGetPointData(BeatmapAssociatedData& beatmapAD, PointDefinition*& anon, const rapidjson::Value& customData, std::string pointName);
ObjectOffset GetObjectOffset(const AnimationObjectData& customData, Track *track, float time);
void OnTrackCreated(Track *track);

} // end namespace AnimationHelper