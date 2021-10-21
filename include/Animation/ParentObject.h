#pragma once
#include "tracks/shared/Animation/Track.h"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "NELogger.h"

DECLARE_CLASS_CODEGEN(TrackParenting, ParentObject, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_SIMPLE_DTOR();
    DECLARE_INSTANCE_METHOD(void, Update);

private:
    Track *track;
    UnityEngine::Transform *origin;
    UnityEngine::Vector3 startPos = UnityEngine::Vector3::get_zero();
    UnityEngine::Quaternion startRot = UnityEngine::Quaternion::get_identity();
    UnityEngine::Quaternion startLocalRot = UnityEngine::Quaternion::get_identity();
    UnityEngine::Vector3 startScale = UnityEngine::Vector3::get_one();
public:
    ~ParentObject();

    std::unordered_set<Track*> childrenTracks;


    void ParentToObject(UnityEngine::Transform *transform);
    void HandleGameObject(Track* track, UnityEngine::GameObject* go, bool removed);

    static void ResetTransformParent(UnityEngine::Transform *transform);
    static void AssignTrack(std::vector<Track*> const& tracks, Track *parentTrack, std::optional<UnityEngine::Vector3> startPos,
        std::optional<UnityEngine::Quaternion> startRot, std::optional<UnityEngine::Quaternion> startLocalRot, std::optional<UnityEngine::Vector3> startScale);
)

class ParentController {
public:
    static inline std::vector<TrackParenting::ParentObject*> parentObjects;

    static TrackParenting::ParentObject *GetParentObjectTrack(Track *track);
    static void OnDestroy();
};