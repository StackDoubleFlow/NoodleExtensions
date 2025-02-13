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
                                                                            std::span<Track* const> tracks,
                                                                            float time) {
  PointDefinition* localDefinitePosition = animationData.definitePosition;



  [[maybe_unused]] bool last;
  std::optional<Vector3> pathDefinitePosition =
      localDefinitePosition ? std::optional(localDefinitePosition->Interpolate(time, last)) : std::nullopt;

  // track animation only
  if (!pathDefinitePosition && !tracks.empty()) {
    if (tracks.size() == 1) {
      Track const* track = tracks.front();
      pathDefinitePosition =
          getPathPropertyNullable<Vector3>(track, track->pathProperties.definitePosition.value, time);
    } else {
      pathDefinitePosition = MSumTrackPathProps(tracks, Vector3::zero(), definitePosition, time);
    }
  }

  if (!pathDefinitePosition) return std::nullopt;

  PointDefinition const* position = animationData.position;
  std::optional<Vector3> pathPosition = position ? std::optional(position->Interpolate(time, last)) : std::nullopt;
  std::optional<Vector3> trackPosition;

  std::optional<Vector3> positionOffset;

  if (!tracks.empty()) {
    if (tracks.size() == 1) {
      Track const* track = tracks.front();

      if (!pathPosition)
        pathPosition = getPathPropertyNullable<Vector3>(track, track->pathProperties.position.value, time);

      trackPosition = getPropertyNullable<Vector3>(track, track->properties.position);
    } else {
      trackPosition = MSumTrackProps(tracks, Vector3::zero(), position);

      if (!pathPosition) pathPosition = MSumTrackPathProps(tracks, Vector3::zero(), position, time);
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

ObjectOffset AnimationHelper::GetObjectOffset(AnimationObjectData const& animationData, std::span<Track* const> tracks,
                                              float time) {
  ObjectOffset offset;

  [[maybe_unused]] bool last;

  PointDefinition const* position = animationData.position;
  PointDefinition const* rotation = animationData.rotation;
  PointDefinition const* scale = animationData.scale;
  PointDefinition const* localRotation = animationData.localRotation;
  PointDefinition const* dissolve = animationData.dissolve;
  PointDefinition const* dissolveArrow = animationData.dissolveArrow;
  PointDefinition const* cuttable = animationData.cuttable;

#define pathPropPointDef(type, name, interpolate)                                                                      \
  std::optional<type> path##name = (name) ? std::optional<type>((name)->interpolate(time, last)) : std::nullopt

#define singlePathProp(type, name, interpolate)                                                                        \
  if (!path##name) path##name = getPathPropertyNullable<type>(track, track->pathProperties.name.value, time)

#define offsetProp(type, name, offsetName, op)                                                                         \
  offset.offsetName = path##name op getPropertyNullable<type>(track, track->properties.name)

  pathPropPointDef(Vector3, position, Interpolate);
  pathPropPointDef(Quaternion, rotation, InterpolateQuaternion);
  pathPropPointDef(Vector3, scale, Interpolate);
  pathPropPointDef(Quaternion, localRotation, InterpolateQuaternion);
  pathPropPointDef(float, dissolve, InterpolateLinear);
  pathPropPointDef(float, dissolveArrow, InterpolateLinear);
  pathPropPointDef(float, cuttable, InterpolateLinear);

  if (!tracks.empty()) {
    if (tracks.size() == 1) {
      auto const track = tracks.front();

      singlePathProp(Vector3, position, Interpolate);
      singlePathProp(Quaternion, rotation, InterpolateQuaternion);
      singlePathProp(Vector3, scale, Interpolate);
      singlePathProp(Quaternion, localRotation, InterpolateQuaternion);
      singlePathProp(float, dissolve, InterpolateLinear);
      singlePathProp(float, dissolveArrow, InterpolateLinear);
      singlePathProp(float, cuttable, InterpolateLinear);

      offsetProp(Vector3, position, positionOffset, +);
      offsetProp(Quaternion, rotation, rotationOffset, *);
      offsetProp(Vector3, scale, scaleOffset, *);
      offsetProp(Quaternion, localRotation, localRotationOffset, *);
      offsetProp(float, dissolve, dissolve, *);
      offsetProp(float, dissolveArrow, dissolveArrow, *);
      offsetProp(float, cuttable, cuttable, *);

    } else {
#define multiPathProp(name, func)                                                                                      \
  if (!path##name) path##name = func

      pathposition = pathposition.has_value() ? pathposition.value() : MSumTrackPathProps(tracks, Vector3::zero(), position, time);
      pathrotation = pathrotation.has_value() ? pathrotation.value() : MMultTrackPathProps(tracks, Quaternion::identity(), rotation, time);
      pathscale = pathscale.has_value() ? pathscale.value() : MMultTrackPathProps(tracks, Vector3::one(), scale, time);
      pathlocalRotation = pathlocalRotation.has_value() ? pathlocalRotation.value() : MMultTrackPathProps(tracks, Quaternion::identity(), localRotation, time);
      pathdissolve = pathdissolve.has_value() ? pathdissolve.value() : MMultTrackPathProps(tracks, 1.0f, dissolve, time);
      pathdissolveArrow = pathdissolveArrow.has_value() ? pathdissolveArrow.value() : MMultTrackPathProps(tracks, 1.0f, dissolveArrow, time);
      pathcuttable = pathcuttable.has_value() ? pathcuttable.value() : MMultTrackPathProps(tracks, 1.0f, cuttable, time);

      offset.positionOffset = pathposition + MSumTrackProps(tracks, Vector3::zero(), position);
      offset.rotationOffset = pathrotation * MMultTrackProps(tracks, Quaternion::identity(), rotation);
      offset.scaleOffset = pathscale * MMultTrackProps(tracks, Vector3::one(), scale);
      offset.localRotationOffset = pathlocalRotation * MMultTrackProps(tracks, Quaternion::identity(), localRotation);
      offset.dissolve = pathdissolve * MMultTrackProps(tracks, 1.0f, dissolve);
      offset.dissolveArrow = pathdissolveArrow * MMultTrackProps(tracks, 1.0f, dissolveArrow);
      offset.cuttable = pathcuttable * MMultTrackProps(tracks, 1.0f, cuttable);
    }
  } else {
    offset.positionOffset = pathposition;
    offset.rotationOffset = pathrotation;
    offset.scaleOffset = pathscale;
    offset.localRotationOffset = pathlocalRotation;
    offset.dissolve = pathdissolve;
    offset.dissolveArrow = pathdissolveArrow;
    offset.cuttable = pathcuttable;
  }

  if (offset.positionOffset)
    offset.positionOffset = offset.positionOffset.value() * NECaches::get_noteLinesDistanceFast();

  if (NECaches::LeftHandedMode) {
    offset.rotationOffset = Animation::MirrorQuaternionNullable(offset.rotationOffset);
    offset.localRotationOffset = Animation::MirrorQuaternionNullable(offset.localRotationOffset);
    offset.positionOffset = Animation::MirrorVectorNullable(offset.positionOffset);
  }

  return offset;
}
