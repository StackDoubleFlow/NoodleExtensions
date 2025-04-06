#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/PointDefinition.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "NELogger.h"
#include "NECaches.h"

using namespace AnimationHelper;
using namespace GlobalNamespace;
using namespace NEVector;
using namespace CustomJSONData;
using namespace Animation;

// BeatmapObjectCallbackController.cpp
extern BeatmapCallbacksController* callbackController;

// Events.cpp
extern BeatmapObjectSpawnController* spawnController;

constexpr std::optional<Vector3> operator+(std::optional<Vector3> const& a, std::optional<Vector3> const& b) {
  if (!a && !b) {
    return std::nullopt;
  }

  Vector3 total = Vector3::zero();
  if (a) {
    total = total + *a;
  }

  if (b) {
    total = total + *b;
  }

  return total;
}

template <typename T> constexpr std::optional<T> operator*(std::optional<T> const& a, std::optional<T> const& b) {
  if (a && b) {
    return *a * *b;
  } else if (a) {
    return a;
  } else if (b) {
    return b;
  } else {
    return std::nullopt;
  }
}

std::optional<NEVector::Vector3> AnimationHelper::GetDefinitePositionOffset(AnimationObjectData const& animationData,
                                                                            std::span<TrackW const> tracks, float time,
                                                                            Tracks::ffi::BaseProviderContext* context) {
  PointDefinitionW localDefinitePosition = animationData.definitePosition;

  [[maybe_unused]] bool last;
  std::optional<Vector3> pathDefinitePosition =
      localDefinitePosition ? std::optional(localDefinitePosition.InterpolateVec3(time, last)) : std::nullopt;

  // track animation only
  if (!pathDefinitePosition && !tracks.empty()) {
    if (tracks.size() == 1) {
      auto track = tracks.front();
      pathDefinitePosition =
          track.GetPathPropertyNamed(PropertyNames::DefinitePosition).InterpolateVec3(time, last, context);
    } else {
      auto positions = Animation::getPathPropertiesVec3(tracks, PropertyNames::DefinitePosition, context, time);
      pathDefinitePosition = Animation::addVector3s(positions);
    }
  }

  if (!pathDefinitePosition) return std::nullopt;

  auto const position = animationData.position;
  std::optional<Vector3> pathPosition = position ? std::optional(position.InterpolateVec3(time, last)) : std::nullopt;
  std::optional<Vector3> trackPosition;

  std::optional<Vector3> positionOffset;

  if (!tracks.empty()) {
    if (tracks.size() == 1) {
      auto track = tracks.front();

      if (!pathPosition)
        pathPosition = track.GetPathPropertyNamed(PropertyNames::Position).InterpolateVec3(time, last, context);

      trackPosition = track.GetPropertyNamed(PropertyNames::Position).GetVec3();
    } else {
      trackPosition = Animation::addVector3s(Animation::getPropertiesVec3(tracks, PropertyNames::Position, {}));

      if (!pathPosition)
        pathPosition =
            Animation::addVector3s(Animation::getPathPropertiesVec3(tracks, PropertyNames::Position, context, time));
    }

    positionOffset = pathPosition + trackPosition;
  } else {
    positionOffset = pathPosition;
  }

  std::optional<Vector3> definitePosition = positionOffset + pathDefinitePosition;
  if (definitePosition) definitePosition = definitePosition.value() * NECaches::get_noteLinesDistanceFast();

  if (NECaches::LeftHandedMode) {
    definitePosition = Animation::MirrorVectorNullable(definitePosition);
  }

  return definitePosition;
}

ObjectOffset AnimationHelper::GetObjectOffset(AnimationObjectData const& animationData, std::span<TrackW const> tracks,
                                              float time, Tracks::ffi::BaseProviderContext* context) {
  ObjectOffset offset;

  [[maybe_unused]] bool last;

  PointDefinitionW position = animationData.position;
  PointDefinitionW rotation = animationData.rotation;
  PointDefinitionW scale = animationData.scale;
  PointDefinitionW localRotation = animationData.localRotation;
  PointDefinitionW dissolve = animationData.dissolve;
  PointDefinitionW dissolveArrow = animationData.dissolveArrow;
  PointDefinitionW cuttable = animationData.cuttable;

  // Get path properties from animation data
  std::optional<Vector3> pathPosition = position ? std::optional(position.InterpolateVec3(time, last)) : std::nullopt;
  std::optional<Quaternion> pathRotation =
      rotation ? std::optional(rotation.InterpolateQuaternion(time, last)) : std::nullopt;
  std::optional<Vector3> pathScale = scale ? std::optional(scale.InterpolateVec3(time, last)) : std::nullopt;
  std::optional<Quaternion> pathLocalRotation =
      localRotation ? std::optional(localRotation.InterpolateQuaternion(time, last)) : std::nullopt;
  std::optional<float> pathDissolve = dissolve ? std::optional(dissolve.InterpolateLinear(time, last)) : std::nullopt;
  std::optional<float> pathDissolveArrow =
      dissolveArrow ? std::optional(dissolveArrow.InterpolateLinear(time, last)) : std::nullopt;
  std::optional<float> pathCuttable = cuttable ? std::optional(cuttable.InterpolateLinear(time, last)) : std::nullopt;

  if (!tracks.empty()) {
    if (tracks.size() == 1) {
      auto const track = tracks.front();

      // Macros to simplify getter code
      if (!pathPosition) pathPosition = track.GetPathPropertyNamed(PropertyNames::Position).InterpolateVec3(time, last, context);
      if (!pathRotation) pathRotation = track.GetPathPropertyNamed(PropertyNames::Rotation).InterpolateQuat(time, last, context);
      if (!pathScale) pathScale = track.GetPathPropertyNamed(PropertyNames::Scale).InterpolateVec3(time, last, context);
      if (!pathLocalRotation)
        pathLocalRotation = track.GetPathPropertyNamed(PropertyNames::LocalRotation).InterpolateQuat(time, last, context);
      
      if (!pathDissolve)
        pathDissolve = track.GetPathPropertyNamed(PropertyNames::Dissolve).InterpolateLinear(time, last, context);
      if (!pathDissolveArrow)
        pathDissolveArrow = track.GetPathPropertyNamed(PropertyNames::DissolveArrow).InterpolateLinear(time, last, context);
      if (!pathCuttable)
        pathCuttable = track.GetPathPropertyNamed(PropertyNames::Cuttable).InterpolateLinear(time, last, context);

      // Combine with track properties
      offset.positionOffset = pathPosition + track.GetPropertyNamed(PropertyNames::Position).GetVec3();
      offset.rotationOffset = pathRotation * track.GetPropertyNamed(PropertyNames::Rotation).GetQuat();
      offset.scaleOffset = pathScale * track.GetPropertyNamed(PropertyNames::Scale).GetVec3();
      offset.localRotationOffset = pathLocalRotation * track.GetPropertyNamed(PropertyNames::LocalRotation).GetQuat();
      offset.dissolve = pathDissolve * track.GetPropertyNamed(PropertyNames::Dissolve).GetFloat();
      offset.dissolveArrow = pathDissolveArrow * track.GetPropertyNamed(PropertyNames::DissolveArrow).GetFloat();
      offset.cuttable = pathCuttable * track.GetPropertyNamed(PropertyNames::Cuttable).GetFloat();
    } else {
      // Multiple tracks - combine their properties
      if (!pathPosition) {
        auto positionPaths = Animation::getPathPropertiesVec3(tracks, PropertyNames::Position, context, time);
        pathPosition = Animation::addVector3s(positionPaths);
      }
      if (!pathRotation) {
        auto rotationPaths = Animation::getPathPropertiesQuat(tracks, PropertyNames::Rotation, context, time);
        pathRotation = Animation::multiplyQuaternions(rotationPaths);
      }
      if (!pathScale) {
        auto scalePaths = Animation::getPathPropertiesVec3(tracks, PropertyNames::Scale, context, time);
        pathScale = Animation::multiplyVector3s(scalePaths);
      }
      if (!pathLocalRotation) {
        auto localRotationPaths = Animation::getPathPropertiesQuat(tracks, PropertyNames::LocalRotation, context, time);
        pathLocalRotation = Animation::multiplyQuaternions(localRotationPaths);
      }
      if (!pathDissolve) {
        auto dissolvePaths = Animation::getPathPropertiesFloat(tracks, PropertyNames::Dissolve, context, time);
        pathDissolve = Animation::multiplyFloats(dissolvePaths);
      }
      if (!pathDissolveArrow) {
        auto dissolveArrowPaths =
            Animation::getPathPropertiesFloat(tracks, PropertyNames::DissolveArrow, context, time);
        pathDissolveArrow = Animation::multiplyFloats(dissolveArrowPaths);
      }
      if (!pathCuttable) {
        auto cuttablePaths = Animation::getPathPropertiesFloat(tracks, PropertyNames::Cuttable, context, time);
        pathCuttable = Animation::multiplyFloats(cuttablePaths);
      }

      // Combine track properties with path properties
      auto trackPositions = Animation::getPropertiesVec3(tracks, PropertyNames::Position, {});
      auto trackRotations = Animation::getPropertiesQuat(tracks, PropertyNames::Rotation, {});
      auto trackScales = Animation::getPropertiesVec3(tracks, PropertyNames::Scale, {});
      auto trackLocalRotations = Animation::getPropertiesQuat(tracks, PropertyNames::LocalRotation, {});
      auto trackDissolves = Animation::getPropertiesFloat(tracks, PropertyNames::Dissolve, {});
      auto trackDissolveArrows = Animation::getPropertiesFloat(tracks, PropertyNames::DissolveArrow, {});
      auto trackCuttables = Animation::getPropertiesFloat(tracks, PropertyNames::Cuttable, {});
      
      // Calculate combined track values
      auto combinedTrackPositions = Animation::addVector3s(trackPositions);
      auto combinedTrackRotations = Animation::multiplyQuaternions(trackRotations);
      auto combinedTrackScales = Animation::multiplyVector3s(trackScales);
      auto combinedTrackLocalRotations = Animation::multiplyQuaternions(trackLocalRotations);
      auto combinedTrackDissolves = Animation::multiplyFloats(trackDissolves);
      auto combinedTrackDissolveArrows = Animation::multiplyFloats(trackDissolveArrows);
      auto combinedTrackCuttables = Animation::multiplyFloats(trackCuttables);

      // all paths are non-null by now
      // Set final property values
      offset.positionOffset = pathPosition.value() + combinedTrackPositions;
      offset.rotationOffset = pathRotation.value() * combinedTrackRotations;
      offset.scaleOffset = pathScale.value() * combinedTrackScales;
      offset.localRotationOffset = pathLocalRotation.value() * combinedTrackLocalRotations;
      offset.dissolve = pathDissolve.value() * combinedTrackDissolves;
      offset.dissolveArrow = pathDissolveArrow.value() * combinedTrackDissolveArrows;
      offset.cuttable = pathCuttable.value() * combinedTrackCuttables;
    }
  } else {
    // No tracks - use animation data only
    offset.positionOffset = pathPosition;
    offset.rotationOffset = pathRotation;
    offset.scaleOffset = pathScale;
    offset.localRotationOffset = pathLocalRotation;
    offset.dissolve = pathDissolve;
    offset.dissolveArrow = pathDissolveArrow;
    offset.cuttable = pathCuttable;
  }

  // Apply scale and mirroring
  if (offset.positionOffset)
    offset.positionOffset = offset.positionOffset.value() * NECaches::get_noteLinesDistanceFast();

  if (NECaches::LeftHandedMode) {
    offset.rotationOffset = Animation::MirrorQuaternionNullable(offset.rotationOffset);
    offset.localRotationOffset = Animation::MirrorQuaternionNullable(offset.localRotationOffset);
    offset.positionOffset = Animation::MirrorVectorNullable(offset.positionOffset);
  }

  return offset;
}
