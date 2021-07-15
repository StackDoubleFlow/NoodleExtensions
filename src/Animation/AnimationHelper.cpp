#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/AnimationHelper.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"
#include "NELogger.h"

using namespace AnimationHelper;
using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace CustomJSONData;

// BeatmapObjectCallbackController.cpp
extern BeatmapObjectCallbackController *callbackController;

// Events.cpp
extern BeatmapObjectSpawnController *spawnController;

Vector3 operator*(const Vector3& a, const Vector3& b) {
    return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

std::optional<Vector3> operator+(std::optional<Vector3> a, std::optional<Vector3> b) {
    if (!a && !b) {
        return std::nullopt;
    }

    Vector3 total = Vector3::get_zero();
    if (a) {
        total = total + *a;
    }

    if (b) {
        total = total + *b;
    }

    return total;
}

template<typename T>
std::optional<T> operator*(std::optional<T> a, std::optional<T> b) {
    if (a) {
        if (b) {
            return *a * *b;
        } else {
            return a;
        }
    } else if (b) {
        return b;
    }
    
    return std::nullopt;
}

PointDefinition *AnimationHelper::TryGetPointData(BeatmapAssociatedData& beatmapAD, const rapidjson::Value& customData, std::string pointName) {
    PointDefinition *pointData = nullptr;

    if (!customData.HasMember(pointName.c_str())) return pointData;
    const rapidjson::Value& pointString = customData[pointName.c_str()];

    switch (pointString.GetType()) {
    case rapidjson::kNullType:
        return pointData;
    case rapidjson::kStringType: {
        auto& ad = beatmapAD;
        for (auto const& pair : ad.pointDefinitions) {
        }
        if (ad.pointDefinitions.find(pointString.GetString()) != ad.pointDefinitions.end()) {
            pointData = &ad.pointDefinitions.at(pointString.GetString());
        }
        break;
    }
    default:
        pointData = new PointDefinition(pointString);
        beatmapAD.anonPointDefinitions.push_back(pointData);
    }

    return pointData;
}

std::optional<PointDefinitionInterpolation> GetPathInterpolation(Track *track, std::string name, PropertyType type) {
    return track ? track->pathProperties.FindProperty(name)->value : std::nullopt;
}

std::optional<Quaternion> TryGetQuaternionPathProperty(Track *track, std::string name, float time) {
    std::optional<PointDefinitionInterpolation> pointDataInterpolation = GetPathInterpolation(track, name, PropertyType::quaternion);
    if (pointDataInterpolation) {
        return pointDataInterpolation->InterpolateQuaternion(time);
    }
    return std::nullopt;
}

std::optional<Vector3> TryGetVector3PathProperty(Track *track, std::string name, float time) {
    std::optional<PointDefinitionInterpolation> pointDataInterpolation = GetPathInterpolation(track, name, PropertyType::vector3);
    if (pointDataInterpolation) {
        return pointDataInterpolation->Interpolate(time);
    }
    return std::nullopt;
}

std::optional<float> TryGetLinearPathProperty(Track *track, std::string name, float time) {
    std::optional<PointDefinitionInterpolation> pointDataInterpolation = GetPathInterpolation(track, name, PropertyType::linear);
    if (pointDataInterpolation) {
        return pointDataInterpolation->InterpolateLinear(time);
    }
    return std::nullopt;
}

std::optional<Vector3> AnimationHelper::GetDefinitePositionOffset(const AnimationObjectData& animationData, Track *track, float time) {
    PointDefinition *localDefinitePosition = animationData.definitePosition;

    std::optional<Vector3> pathDefinitePosition = localDefinitePosition ? std::optional{ localDefinitePosition->Interpolate(time) } : TryGetVector3PathProperty(track, "_definitePosition", time);

    if (pathDefinitePosition) {
        PointDefinition *position = animationData.position;
        std::optional<Vector3> pathPosition = position ? std::optional{ position->Interpolate(time) } : TryGetVector3PathProperty(track, "_position", time);
        std::optional<Vector3> trackPosition = track && track->properties.position.value ?
            std::optional{ track->properties.position.value->vector3 } : std::nullopt;
        std::optional<Vector3> positionOffset = pathPosition + trackPosition;
        std::optional<Vector3> definitePosition = positionOffset + pathDefinitePosition;
        if (definitePosition) definitePosition = *definitePosition * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;
        return definitePosition;
    } else {
        return std::nullopt;
    }
}

namespace {
    template<typename T, typename F>
    constexpr auto map(T const& v, F&& func) noexcept {
        return v ? std::optional{ std::invoke(std::forward<F>(func), *v) } : std::nullopt;
    }

    template<typename T, typename F>
    constexpr auto mapTrack(Track *track, std::optional<T> const& v, F&& func) noexcept {
        return track ? map(v, func) : std::nullopt;
    }
}

ObjectOffset AnimationHelper::GetObjectOffset(const AnimationObjectData& animationData, Track *track, float time) {
    ObjectOffset offset;

    PointDefinition *position = animationData.position;
    PointDefinition *rotation = animationData.rotation;
    PointDefinition *scale = animationData.scale;
    PointDefinition *localRotation = animationData.localRotation;
    PointDefinition *dissolve = animationData.dissolve;
    PointDefinition *dissolveArrow = animationData.dissolveArrow;
    PointDefinition *cuttable = animationData.cuttable;

    std::optional<Vector3> pathPosition = position ? std::optional{ position->Interpolate(time) } : TryGetVector3PathProperty(track, "_position", time);
    std::optional<Quaternion> pathRotation = rotation ? std::optional{ rotation->InterpolateQuaternion(time) } : TryGetQuaternionPathProperty(track, "_rotation", time);
    std::optional<Vector3> pathScale = scale ? std::optional{ scale->Interpolate(time) } : TryGetVector3PathProperty(track, "_scale", time);
    std::optional<Quaternion> pathLocalRotation = localRotation ? std::optional{ localRotation->InterpolateQuaternion(time) } : TryGetQuaternionPathProperty(track, "_localRotation", time);
    std::optional<float> pathDissolve = dissolve ? std::optional{ dissolve->InterpolateLinear(time) } : TryGetLinearPathProperty(track, "_dissolve", time);
    std::optional<float> pathDissolveArrow = dissolveArrow ? std::optional{ dissolveArrow->InterpolateLinear(time) } : TryGetLinearPathProperty(track, "_dissolveArrow", time);
    std::optional<float> pathCuttable = cuttable ? std::optional{ cuttable->InterpolateLinear(time) } : TryGetLinearPathProperty(track, "_cuttable", time);

    offset.positionOffset = pathPosition + mapTrack(track, track->properties.position.value, [](auto const& p) { return p.vector3; });
    if (offset.positionOffset) offset.positionOffset = *offset.positionOffset * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

    offset.rotationOffset = pathRotation * mapTrack(track, track->properties.rotation.value, [](auto const& p) { return p.quaternion; });
    offset.scaleOffset = pathScale * mapTrack(track, track->properties.scale.value, [](auto const& p) { return p.vector3; });
    offset.localRotationOffset = pathLocalRotation * mapTrack(track, track->properties.localRotation.value, [](auto const& p) { return p.quaternion; });
    offset.dissolve = pathDissolve * mapTrack(track, track->properties.dissolve.value, [](auto const& p) { return p.linear; });
    offset.dissolveArrow = pathDissolveArrow * mapTrack(track, track->properties.dissolveArrow.value, [](auto const& p) { return p.linear; });
    offset.cuttable = pathCuttable * mapTrack(track, track->properties.cuttable.value, [](auto const& p) { return p.linear; });

    return offset;
}

void AnimationHelper::OnTrackCreated(Track *track) {

}