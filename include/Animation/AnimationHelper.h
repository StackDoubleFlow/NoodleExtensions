#pragma once
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "tracks/shared/Animation/Track.h"

#include "tracks/shared/Animation/Animation.h"
#include "AssociatedData.h"


// why not?
template<typename T>
inline constexpr std::optional<T> getPropertyNullableFast(Track* track, const Property& prop, uint64_t lastCheckedTime) {
    if (lastCheckedTime != 0 && prop.lastUpdated != 0 && prop.lastUpdated < lastCheckedTime) return std::nullopt;

    auto ret = Animation::getPropertyNullable<T>(track, prop.value);

    if (NECaches::LeftHandedMode) {
        if constexpr(std::is_same_v<T, NEVector::Vector3>) {
            return Animation::MirrorVectorNullable(ret);
        }

        if constexpr(std::is_same_v<T, NEVector::Quaternion>) {
            return Animation::MirrorQuaternionNullable(ret);
        }
    }

    return ret;
}

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

std::optional<NEVector::Vector3> GetDefinitePositionOffset(const AnimationObjectData& animationData, std::span<Track *> tracks, float time);
ObjectOffset GetObjectOffset(const AnimationObjectData& customData, std::span<Track *> tracks, float time);

} // end namespace AnimationHelper