#pragma once
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

#include "tracks/shared/Animation/Track.h"
#include "AssociatedData.h"

// TODO: Move these utils to Tracks proper, both Chroma and NE use it
#pragma region track_utils

// Ok Stack, why the hell does prop param nullptr in the method if track is null, but then if you null-check track in the method it just works:tm:
// C++ compiler tomfoolery that's above my pay grade, that's what this is
// my most educated guess is compiler inlining method magic
template<typename T>
static constexpr std::optional<T> getPropertyNullable(Track* track, const std::optional<PropertyValue>& prop) {
    if (!track) return std::nullopt;
    if (!prop) return std::nullopt;

    // TODO: Left handed

    //    float linear;
    //    NEVector::Vector3 vector3;
    //    NEVector::Vector4 vector4;
    //    NEVector::Quaternion quaternion;

    if constexpr(std::is_same_v<T, float>) {
        return prop.value().linear;
    } else if constexpr(std::is_same_v<T, NEVector::Vector3>) {
        return prop.value().vector3;
    } else if constexpr(std::is_same_v<T, NEVector::Vector4>) {
        return prop.value().vector4;
    } else if constexpr(std::is_same_v<T, NEVector::Quaternion>) {
        return prop.value().quaternion;
    }

    return std::nullopt;
}

// why not?
template<typename T>
static constexpr std::optional<T> getPropertyNullable(Track* track, const Property& prop) {
    return getPropertyNullable<T>(track, prop.value);
}

template<typename T>
static constexpr std::optional<T> getPathPropertyNullable(Track* track, std::optional<PointDefinitionInterpolation>& prop, float time) {
    if (!track) return std::nullopt;
    if (!prop) return std::nullopt;

    // TODO: Left handed

    //    float linear;
    //    NEVector::Vector3 vector3;
    //    NEVector::Vector4 vector4;
    //    NEVector::Quaternion quaternion;

    if constexpr(std::is_same_v<T, float>) {
        return prop.value().InterpolateLinear(time);
    } else if constexpr(std::is_same_v<T, NEVector::Vector3>) {
        return prop.value().Interpolate(time);
    } else if constexpr(std::is_same_v<T, NEVector::Vector4>) {
        return prop.value().InterpolateVector4(time);
    } else if constexpr(std::is_same_v<T, NEVector::Quaternion>) {
        return prop.value().InterpolateQuaternion(time);
    }

    return std::nullopt;
}

template<typename T>
static std::optional<T> MultiTrackPathProps(std::vector<Track*> const& tracks, T const& defaultT, float time, std::function<std::optional<PointDefinitionInterpolation> (Track*)> const& vectorExpression) {
    static_assert(std::is_same_v<T, float> ||
                          std::is_same_v<T, NEVector::Vector3> ||
                          std::is_same_v<T, NEVector::Vector4> ||
                          std::is_same_v<T, NEVector::Quaternion>, "Not valid type");

    bool valid = false;
    T total = defaultT;

    for (auto& track : tracks) {
        auto point = vectorExpression(track);
        auto result = getPathPropertyNullable<T>(track, point, time);

        if (result) {
            total = result.value() * total;
            valid = true;
        }
    }

    return valid ? std::make_optional(total) : std::nullopt;
}

template<typename T>
static std::optional<T> SumTrackPathProps(std::vector<Track*> const& tracks, T const& defaultT, float time, std::function<std::optional<PointDefinitionInterpolation> (Track*)> const& vectorExpression) {
    static_assert(std::is_same_v<T, float> ||
                  std::is_same_v<T, NEVector::Vector3> ||
                  std::is_same_v<T, NEVector::Vector4> ||
                  std::is_same_v<T, NEVector::Quaternion>, "Not valid type");

    bool valid = false;
    T total = defaultT;

    for (auto& track : tracks) {
        auto point = vectorExpression(track);
        auto result = getPathPropertyNullable<T>(track, point, time);

        if (result) {
            total = result.value() + total;
            valid = true;
        }
    }

    return valid ? std::make_optional(total) : std::nullopt;
}

template<typename T>
static std::optional<T> MultiTrackProps(std::vector<Track*> const& tracks, T const& defaultT, std::function<std::optional<PropertyValue> (Track*)> const& vectorExpression) {
    static_assert(std::is_same_v<T, float> ||
                  std::is_same_v<T, NEVector::Vector3> ||
                  std::is_same_v<T, NEVector::Vector4> ||
                  std::is_same_v<T, NEVector::Quaternion>, "Not valid type");

    bool valid = false;
    T total = defaultT;

    for (auto& track : tracks) {
        auto point = vectorExpression(track);
        auto result = getPropertyNullable<T>(track, point);

        if (result) {
            total = result.value() * total;
            valid = true;
        }
    }

    return valid ? std::make_optional(total) : std::nullopt;
}

template<typename T>
static std::optional<T> SumTrackProps(std::vector<Track*> const& tracks, T const& defaultT, std::function<std::optional<PropertyValue> (Track*)> const& vectorExpression) {
    static_assert(std::is_same_v<T, float> ||
                  std::is_same_v<T, NEVector::Vector3> ||
                  std::is_same_v<T, NEVector::Vector4> ||
                  std::is_same_v<T, NEVector::Quaternion>, "Not valid type");

    bool valid = false;
    T total = defaultT;

    for (auto& track : tracks) {
        auto point = vectorExpression(track);
        auto result = getPropertyNullable<T>(track, point);

        if (result) {
            total = result.value() + total;
            valid = true;
        }
    }

    return valid ? std::make_optional(total) : std::nullopt;
}

#define MSumTrackProps(tracks, defaultT, prop) SumTrackProps(tracks, defaultT, [](Track* track) { return track->properties.prop.value;})
#define MMultTrackProps(tracks, defaultT, prop) MultiTrackProps(tracks, defaultT, [](Track* track) { return track->properties.prop.value; })

#define MSumTrackPathProps(tracks, defaultT, time, prop) SumTrackPathProps(tracks, defaultT, time, [](Track* track) { return track->pathProperties.prop.value; })
#define MMultTrackPathProps(tracks, defaultT, time, prop) MultiTrackPathProps(tracks, defaultT, time, [](Track* track) { return track->pathProperties.prop.value; })

#pragma endregion

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

std::optional<NEVector::Vector3> GetDefinitePositionOffset(const AnimationObjectData& animationData, std::vector<Track *> tracks, float time);
ObjectOffset GetObjectOffset(const AnimationObjectData& customData, std::vector<Track *> tracks, float time);

} // end namespace AnimationHelper