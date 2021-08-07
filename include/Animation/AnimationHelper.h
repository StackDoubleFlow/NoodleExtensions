#pragma once
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "tracks/shared/Animation/Track.h"
#include "AssociatedData.h"

namespace AnimationHelper {
struct ObjectOffset {
    std::optional<NEVector::Vector3> positionOffset;
    std::optional<NEVector::Quaternion> rotationOffset;
    std::optional<NEVector::Vector3> scaleOffset;
    std::optional<NEVector::Quaternion> localRotationOffset;
    std::optional<float> dissolve;
    std::optional<float> dissolveArrow;
    std::optional<float> cuttable;
};

std::optional<NEVector::Vector3> GetDefinitePositionOffset(const AnimationObjectData& animationData, Track *track, float time);
ObjectOffset GetObjectOffset(const AnimationObjectData& customData, Track *track, float time);

} // end namespace AnimationHelper