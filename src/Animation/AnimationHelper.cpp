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

    std::optional<Vector3> trackPosition = track && track->properties.position.value ?
        std::optional{ track->properties.position.value->vector3 } : std::nullopt;
    offset.positionOffset = pathPosition + trackPosition;
    if (offset.positionOffset) offset.positionOffset = *offset.positionOffset * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

    std::optional<Quaternion> trackRotation = track && track->properties.rotation.value ?
        std::optional{ track->properties.rotation.value->quaternion } : std::nullopt;
    offset.rotationOffset = pathRotation * trackRotation;

    std::optional<Vector3> trackScale = track && track->properties.scale.value ?
        std::optional{ track->properties.scale.value->vector3 } : std::nullopt;
    offset.scaleOffset = pathScale * trackScale;

    std::optional<Quaternion> trackLocalRotation = track && track->properties.localRotation.value ?
        std::optional{ track->properties.localRotation.value->quaternion } : std::nullopt;
    offset.localRotationOffset = pathLocalRotation * trackLocalRotation;

    std::optional<float> trackDissolve = track && track->properties.dissolve.value ?
        std::optional{ track->properties.dissolve.value->linear } : std::nullopt;
    offset.dissolve = pathDissolve * trackDissolve;

    std::optional<float> trackDissolveArrow = track && track->properties.dissolveArrow.value ?
        std::optional{ track->properties.dissolveArrow.value->linear } : std::nullopt;
    offset.dissolveArrow = pathDissolveArrow * trackDissolveArrow;

    std::optional<float> trackCuttable = track && track->properties.cuttable.value ?
        std::optional{ track->properties.cuttable.value->linear } : std::nullopt;
    offset.cuttable = pathCuttable * trackCuttable;

    return offset;
}

void AnimationHelper::OnTrackCreated(Track *track) {
    // These are all done in the constructors now

    // track->properties.position = Property(PropertyType::vector3);
    // track->properties.rotation = Property(PropertyType::quaternion);
    // track->properties.scale = Property(PropertyType::vector3);
    // track->properties.localRotation = Property(PropertyType::quaternion);
    // track->properties.dissolve = Property(PropertyType::linear);
    // track->properties.dissolveArrow = Property(PropertyType::linear);
    // track->properties.time = Property(PropertyType::linear);
    // track->properties.cuttable = Property(PropertyType::linear);

    // track->pathProperties.position = PathProperty(PropertyType::vector3);
    // track->pathProperties.rotation = PathProperty(PropertyType::quaternion);
    // track->pathProperties.scale = PathProperty(PropertyType::vector3);
    // track->pathProperties.localRotation = PathProperty(PropertyType::quaternion);
    // track->pathProperties.dissolve = PathProperty(PropertyType::linear);
    // track->pathProperties.dissolveArrow = PathProperty(PropertyType::linear);
    // track->pathProperties.cuttable = PathProperty(PropertyType::linear);
}