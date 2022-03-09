#pragma once
#include "tracks/shared/Animation/Track.h"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "NELogger.h"
#include "tracks/shared/Vector.h"

#include "AssociatedData.h"

DECLARE_CLASS_CODEGEN(TrackParenting, ParentObject, UnityEngine::MonoBehaviour,
    DECLARE_DEFAULT_CTOR();
    DECLARE_SIMPLE_DTOR();
    DECLARE_INSTANCE_METHOD(void, Update);

private:
    Track *track;
    UnityEngine::Transform *origin;
    NEVector::Vector3 startPos = NEVector::Vector3::zero();
    NEVector::Quaternion startRot = NEVector::Quaternion::identity();
    NEVector::Quaternion startLocalRot = NEVector::Quaternion::identity();
    NEVector::Vector3 startScale = NEVector::Vector3::one();
    bool worldPositionStays;
public:
    ~ParentObject();

    std::unordered_set<Track*> childrenTracks;


    void ParentToObject(UnityEngine::Transform *transform);
    void HandleGameObject(Track* track, UnityEngine::GameObject* go, bool removed);

    static void ResetTransformParent(UnityEngine::Transform *transform);
    static void AssignTrack(ParentTrackEventData const& parentTrackEventData);
)

class ParentController {
public:
    static inline std::vector<TrackParenting::ParentObject*> parentObjects;

    static TrackParenting::ParentObject *GetParentObjectTrack(Track *track);
    static void OnDestroy();
};