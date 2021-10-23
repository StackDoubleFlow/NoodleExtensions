#pragma once
#include "tracks/shared/Animation/Track.h"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "NELogger.h"

namespace NEVector {
    struct Vector3;
    struct Quaternion;
}

DECLARE_CLASS_CODEGEN(TrackParenting, ParentObject, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_SIMPLE_DTOR();
    DECLARE_INSTANCE_METHOD(void, Update);

private:
    Track *track;
    UnityEngine::Transform *origin;
    NEVector::Vector3 startPos = NEVector::Vector3::get_zero();
    NEVector::Quaternion startRot = NEVector::Quaternion::get_identity();
    NEVector::Quaternion startLocalRot = NEVector::Quaternion::get_identity();
    NEVector::Vector3 startScale = NEVector::Vector3::get_one();
public:
    ~ParentObject();

    std::unordered_set<Track*> childrenTracks;


    void ParentToObject(UnityEngine::Transform *transform);
    void HandleGameObject(Track* track, UnityEngine::GameObject* go, bool removed);

    static void ResetTransformParent(UnityEngine::Transform *transform);
    static void AssignTrack(std::vector<Track*> const& tracks, Track *parentTrack, std::optional<NEVector::Vector3> startPos,
        std::optional<NEVector::Quaternion> startRot, std::optional<NEVector::Quaternion> startLocalRot, std::optional<NEVector::Vector3> startScale);
)

class ParentController {
public:
    static inline std::vector<TrackParenting::ParentObject*> parentObjects;

    static TrackParenting::ParentObject *GetParentObjectTrack(Track *track);
    static void OnDestroy();
};