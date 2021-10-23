#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/PointDefinition.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "NELogger.h"

using namespace AnimationHelper;
using namespace GlobalNamespace;
using namespace NEVector;
using namespace CustomJSONData;

// BeatmapObjectCallbackController.cpp
extern BeatmapObjectCallbackController *callbackController;

// Events.cpp
extern BeatmapObjectSpawnController *spawnController;

std::optional<Vector3> operator+(std::optional<Vector3> a, std::optional<Vector3> b) {
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

template<typename T>
std::optional<T> operator*(std::optional<T> a, std::optional<T> b) {
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

std::optional<NEVector::Vector3> AnimationHelper::GetDefinitePositionOffset(const AnimationObjectData& animationData, std::vector<Track *> tracks, float time) {
    PointDefinition *localDefinitePosition = animationData.definitePosition;

    std::optional<Vector3> pathDefinitePosition;
    std::optional<Vector3> definitePosition;
    std::optional<Vector3> pathPosition;
    std::optional<Vector3> trackPosition;

    if (tracks.size() == 1) {
        Track *track = tracks.front();

        pathDefinitePosition = localDefinitePosition ? std::optional{
                localDefinitePosition->Interpolate(time)} : getPathPropertyNullable<Vector3>(track,
                                                                                             track->pathProperties.definitePosition.value,
                                                                                             time);

        if (pathDefinitePosition) {
            PointDefinition *position = animationData.position;
            pathPosition = position ? std::optional{position->Interpolate(time)}
                                    : getPathPropertyNullable<Vector3>(track,
                                                                       track->pathProperties.position.value,
                                                                       time);
            trackPosition = getPropertyNullable<Vector3>(track,
                                                         track->properties.position.value);


        } else {
            return std::nullopt;
        }
    } else {
        pathDefinitePosition = localDefinitePosition ? std::optional{
                localDefinitePosition->Interpolate(time)} : MSumTrackPathProps(tracks, 1.0f, time, definitePosition);

        if (pathDefinitePosition) {
            PointDefinition *position = animationData.position;
            pathPosition = position ? std::optional{position->Interpolate(time)}
                                    : MSumTrackPathProps(tracks, Vector3::zero(), time, position);
            trackPosition = MSumTrackPathProps(tracks, Vector3::zero(), time, position);
        } else {
            return std::nullopt;
        }
    }

    std::optional<Vector3> positionOffset = pathPosition + trackPosition;
    definitePosition = positionOffset + pathDefinitePosition;
    if (definitePosition)
        definitePosition = *definitePosition *
                           spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

    return definitePosition;
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

ObjectOffset AnimationHelper::GetObjectOffset(const AnimationObjectData& animationData, std::vector<Track *> tracks, float time) {
    ObjectOffset offset;

    PointDefinition *position = animationData.position;
    PointDefinition *rotation = animationData.rotation;
    PointDefinition *scale = animationData.scale;
    PointDefinition *localRotation = animationData.localRotation;
    PointDefinition *dissolve = animationData.dissolve;
    PointDefinition *dissolveArrow = animationData.dissolveArrow;
    PointDefinition *cuttable = animationData.cuttable;

    if (tracks.size() == 1) {
        auto track = tracks.front();
        std::optional<Vector3> pathPosition = position ? std::optional{position->Interpolate(time)}
                                                       : getPathPropertyNullable<Vector3>(track,
                                                                                          track->pathProperties.position.value,
                                                                                          time);
        std::optional<Quaternion> pathRotation = rotation ? std::optional{rotation->InterpolateQuaternion(time)}
                                                          : getPathPropertyNullable<Quaternion>(track,
                                                                                                track->pathProperties.rotation.value,
                                                                                                time);
        std::optional<Vector3> pathScale = scale ? std::optional{scale->Interpolate(time)}
                                                 : getPathPropertyNullable<Vector3>(track,
                                                                                    track->pathProperties.scale.value,
                                                                                    time);
        std::optional<Quaternion> pathLocalRotation = localRotation ? std::optional{
                localRotation->InterpolateQuaternion(time)} : getPathPropertyNullable<Quaternion>(track,
                                                                                                  track->pathProperties.localRotation.value,
                                                                                                  time);
        std::optional<float> pathDissolve = dissolve ? std::optional{dissolve->InterpolateLinear(time)}
                                                     : getPathPropertyNullable<float>(track,
                                                                                      track->pathProperties.dissolve.value,
                                                                                      time);
        std::optional<float> pathDissolveArrow = dissolveArrow ? std::optional{dissolveArrow->InterpolateLinear(time)}
                                                               : getPathPropertyNullable<float>(track,
                                                                                                track->pathProperties.dissolveArrow.value,
                                                                                                time);
        std::optional<float> pathCuttable = cuttable ? std::optional{cuttable->InterpolateLinear(time)}
                                                     : getPathPropertyNullable<float>(track,
                                                                                      track->pathProperties.cuttable.value,
                                                                                      time);

        offset.positionOffset = pathPosition + mapTrack(track, track->properties.position.value,
                                                        [](auto const &p) { return p.vector3; });


        offset.rotationOffset = pathRotation * mapTrack(track, track->properties.rotation.value,
                                                        [](auto const &p) { return p.quaternion; });
        offset.scaleOffset =
                pathScale * mapTrack(track, track->properties.scale.value, [](auto const &p) { return p.vector3; });
        offset.localRotationOffset = pathLocalRotation * mapTrack(track, track->properties.localRotation.value,
                                                                  [](auto const &p) { return p.quaternion; });
        offset.dissolve = pathDissolve *
                          mapTrack(track, track->properties.dissolve.value, [](auto const &p) { return p.linear; });
        offset.dissolveArrow = pathDissolveArrow * mapTrack(track, track->properties.dissolveArrow.value,
                                                            [](auto const &p) { return p.linear; });
        offset.cuttable = pathCuttable *
                          mapTrack(track, track->properties.cuttable.value, [](auto const &p) { return p.linear; });

    } else {
        std::optional<Vector3> pathPosition = position ? std::optional{position->Interpolate(time)}
                                                       : MSumTrackPathProps(tracks, Vector3::zero(), time, position);

        std::optional<Quaternion> pathRotation = rotation ? std::optional{rotation->InterpolateQuaternion(time)}
                                                          : MMultTrackPathProps(tracks, Quaternion::identity(), time,
                                                                                rotation);
        std::optional<Vector3> pathScale = scale ? std::optional{scale->Interpolate(time)}
                                                 : MMultTrackPathProps(tracks, Vector3::zero(), time, scale);
        std::optional<Quaternion> pathLocalRotation = localRotation ? std::optional{
                localRotation->InterpolateQuaternion(time)} : MMultTrackPathProps(tracks, Quaternion::identity(), time,
                                                                                  localRotation);

        std::optional<float> pathDissolve = dissolve ? std::optional{dissolve->InterpolateLinear(time)}
                                                     : MMultTrackPathProps(tracks, 1.0f, time, dissolve);

        std::optional<float> pathDissolveArrow = dissolveArrow ? std::optional{dissolveArrow->InterpolateLinear(time)}
                                                               : MMultTrackPathProps(tracks, 1.0f, time, dissolveArrow);

        std::optional<float> pathCuttable = cuttable ? std::optional{cuttable->InterpolateLinear(time)}
                                                     : MMultTrackPathProps(tracks, 1.0f, time, cuttable);

        offset.positionOffset = pathPosition + MSumTrackProps(tracks, Vector3::zero(), position);


        offset.rotationOffset = pathRotation * MMultTrackProps(tracks, Quaternion::identity(), rotation);
        offset.scaleOffset = pathScale * MSumTrackProps(tracks, Vector3::zero(), scale);
        offset.localRotationOffset = pathLocalRotation * MMultTrackProps(tracks, Quaternion::identity(), localRotation);
        offset.dissolve = pathDissolve * MMultTrackProps(tracks, 1.0f, dissolve);
        offset.dissolveArrow = pathDissolveArrow * MMultTrackProps(tracks, 1.0f, dissolveArrow);
        offset.cuttable = pathCuttable * MMultTrackProps(tracks, 1.0f, cuttable);
    }

    if (offset.positionOffset)
        offset.positionOffset = *offset.positionOffset *
                                spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

    return offset;
}
