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

Vector3 vmult(Vector3 a, Vector3 b) {
    return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

std::optional<Vector3> vsumNullable(std::optional<Vector3> a, std::optional<Vector3> b) {
    if (!a.has_value() && !b.has_value()) {
        return std::nullopt;
    }

    Vector3 total = Vector3::get_zero();
    if (a.has_value()) {
        total = total + *a;
    }

    if (b.has_value()) {
        total = total + *b;
    }

    return total;
}

std::optional<Vector3> vmultNullable(std::optional<Vector3> a, std::optional<Vector3> b) {
    if (a.has_value()) {
        if (b.has_value()) {
            return vmult(*a, *b);
        } else {
            return a;
        }
    } else if (b.has_value()) {
        return b;
    }
    
    return std::nullopt;
}

std::optional<Quaternion> qmultNullable(std::optional<Quaternion> a, std::optional<Quaternion> b) {
    if (a.has_value()) {
        if (b.has_value()) {
            return *a * *b;
        } else {
            return a;
        }
    } else if (b.has_value()) {
        return b;
    }
    
    return std::nullopt;
}

std::optional<float> fmultNullable(std::optional<float> a, std::optional<float> b) {
    if (a.has_value()) {
        if (b.has_value()) {
            return *a * *b;
        } else {
            return a;
        }
    } else if (b.has_value()) {
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
    if (pointDataInterpolation.has_value()) {
        return pointDataInterpolation->InterpolateQuaternion(time);
    }
    return std::nullopt;
}

std::optional<Vector3> TryGetVector3PathProperty(Track *track, std::string name, float time) {
    std::optional<PointDefinitionInterpolation> pointDataInterpolation = GetPathInterpolation(track, name, PropertyType::vector3);
    if (pointDataInterpolation.has_value()) {
        return pointDataInterpolation->Interpolate(time);
    }
    return std::nullopt;
}

std::optional<float> TryGetLinearPathProperty(Track *track, std::string name, float time) {
    std::optional<PointDefinitionInterpolation> pointDataInterpolation = GetPathInterpolation(track, name, PropertyType::linear);
    if (pointDataInterpolation.has_value()) {
        return pointDataInterpolation->InterpolateLinear(time);
    }
    return std::nullopt;
}

std::optional<Vector3> AnimationHelper::GetDefinitePositionOffset(const AnimationObjectData& animationData, Track *track, float time) {
    PointDefinition *localDefinitePosition = animationData.definitePosition;

    std::optional<Vector3> pathDefinitePosition = localDefinitePosition ? std::optional{ localDefinitePosition->Interpolate(time) } : TryGetVector3PathProperty(track, "_definitePosition", time);

    if (pathDefinitePosition.has_value()) {
        PointDefinition *position = animationData.position;
        std::optional<Vector3> pathPosition = position ? std::optional{ position->Interpolate(time) } : TryGetVector3PathProperty(track, "_position", time);
        std::optional<Vector3> trackPosition = track && track->properties.position.value.has_value() ?
        std::optional{ track->properties.position.value->vector3 } : std::nullopt;
        std::optional<Vector3> positionOffset = vsumNullable(pathPosition, trackPosition);
        std::optional<Vector3> definitePosition = vsumNullable(positionOffset, pathDefinitePosition);
        if (definitePosition) definitePosition = *definitePosition * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;
        return definitePosition;
    } else {
        return std::nullopt;
    }
}


ObjectOffset AnimationHelper::GetObjectOffset(const AnimationObjectData& animationData, Track *track, float time) {
    ObjectOffset offset;

    // getBeatmapAD(((CustomJSONData::CustomBeatmapData*) callbackController->beatmapData)->customData);

    PointDefinition *position = animationData.position;
    std::optional<Vector3> pathPosition = position ? std::optional{ position->Interpolate(time) } : TryGetVector3PathProperty(track, "_position", time);
    std::optional<Vector3> trackPosition = track && track->properties.position.value.has_value() ?
        std::optional{ track->properties.position.value->vector3 } : std::nullopt;
    offset.positionOffset = vsumNullable(pathPosition, trackPosition);
    if (offset.positionOffset) offset.positionOffset = *offset.positionOffset * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

    PointDefinition *rotation = animationData.rotation;
    std::optional<Quaternion> pathRotation = rotation ? std::optional{ rotation->InterpolateQuaternion(time) } : TryGetQuaternionPathProperty(track, "_rotation", time);
    std::optional<Quaternion> trackRotation = track && track->properties.rotation.value.has_value() ?
        std::optional{ track->properties.rotation.value->quaternion } : std::nullopt;
    offset.rotationOffset = qmultNullable(pathRotation, trackRotation);

    PointDefinition *scale = animationData.scale;
    std::optional<Vector3> pathScale = scale ? std::optional{ scale->Interpolate(time) } : TryGetVector3PathProperty(track, "_scale", time);
    std::optional<Vector3> trackScale = track && track->properties.scale.value.has_value() ?
        std::optional{ track->properties.scale.value->vector3 } : std::nullopt;
    offset.scaleOffset = vmultNullable(pathScale, trackScale);

    PointDefinition *localRotation = animationData.localRotation;
    std::optional<Quaternion> pathLocalRotation = localRotation ? std::optional{ localRotation->InterpolateQuaternion(time) } : TryGetQuaternionPathProperty(track, "_localRotation", time);
    std::optional<Quaternion> trackLocalRotation = track && track->properties.localRotation.value.has_value() ?
        std::optional{ track->properties.localRotation.value->quaternion } : std::nullopt;
    offset.localRotationOffset = qmultNullable(pathLocalRotation, trackLocalRotation);

    PointDefinition *dissolve = animationData.dissolve;
    std::optional<float> pathDissolve = dissolve ? std::optional{ dissolve->InterpolateLinear(time) } : TryGetLinearPathProperty(track, "_dissolve", time);
    std::optional<float> trackDissolve = track && track->properties.dissolve.value.has_value() ?
        std::optional{ track->properties.dissolve.value->linear } : std::nullopt;
    offset.dissolve = fmultNullable(pathDissolve, trackDissolve);

    return offset;
}

void AnimationHelper::OnTrackCreated(Track *track) {
    track->properties.position = Property(PropertyType::vector3);
    track->properties.rotation = Property(PropertyType::quaternion);
    track->properties.scale = Property(PropertyType::vector3);
    track->properties.localRotation = Property(PropertyType::quaternion);
    track->properties.dissolve = Property(PropertyType::linear);
    track->properties.dissolveArrow = Property(PropertyType::linear);
    track->properties.time = Property(PropertyType::linear);
    track->properties.cuttable = Property(PropertyType::linear);

    track->pathProperties.position = PathProperty(PropertyType::vector3);
    track->pathProperties.rotation = PathProperty(PropertyType::quaternion);
    track->pathProperties.scale = PathProperty(PropertyType::vector3);
    track->pathProperties.localRotation = PathProperty(PropertyType::quaternion);
    track->pathProperties.dissolve = PathProperty(PropertyType::linear);
    track->pathProperties.dissolveArrow = PathProperty(PropertyType::linear);
    track->pathProperties.cuttable = PathProperty(PropertyType::linear);
}