#pragma once
#include "Animation/Track.h"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "NELogger.h"

namespace GlobalNamespace {
    class PauseController;
}

DECLARE_CLASS_CODEGEN(TrackParenting, PlayerTrack, UnityEngine::MonoBehaviour, 
    DECLARE_CTOR(ctor);
    DECLARE_METHOD(void, Update);
    DECLARE_METHOD(void, OnDestroy);

    REGISTER_FUNCTION(
        NELogger::GetLogger().debug("Registering PlayerTrack!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Update);
        REGISTER_METHOD(OnDestroy);
    )
public:
    static void AssignTrack(Track *track);
private:
    static inline PlayerTrack *instance;
    static inline Track *track;
    static inline UnityEngine::Vector3 startPos;
    static inline UnityEngine::Quaternion startRot;
    static inline UnityEngine::Quaternion startLocalRot;
    static inline UnityEngine::Vector3 startScale;
    static inline UnityEngine::Transform *origin;
    static inline GlobalNamespace::PauseController *pauseController;

    static void OnDidPauseEvent();
)
