#include "Animation/PlayerTrack.h"
#include "Animation/AnimationHelper.h"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "System/Action.hpp"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace System;

// Events.cpp
extern BeatmapObjectSpawnController *spawnController;

static Action *didPauseEventAction;

DEFINE_TYPE(TrackParenting, PlayerTrack);

void PlayerTrack::ctor() {
    startPos = NEVector::Vector3::get_zero();
    startRot = NEVector::Quaternion::get_identity();
    startLocalRot = NEVector::Quaternion::get_identity();
    startScale = NEVector::Vector3::get_one();
}

void PlayerTrack::AssignTrack(Track *track) {
    if (!instance) {
        GameObject *gameObject = GameObject::Find(il2cpp_utils::newcsstr("LocalPlayerGameCore"));
        GameObject *noodleObject = GameObject::New_ctor(il2cpp_utils::newcsstr("NoodlePlayerTrack"));
        origin = noodleObject->get_transform();
        origin->SetParent(gameObject->get_transform()->get_parent(), true);
        gameObject->get_transform()->SetParent(origin, true);

        instance = noodleObject->AddComponent<PlayerTrack*>();
        pauseController = Object::FindObjectOfType<PauseController*>();
        if (pauseController) {
            didPauseEventAction = il2cpp_utils::MakeAction<Action*>(PlayerTrack::OnDidPauseEvent);
            pauseController->add_didPauseEvent(didPauseEventAction);
        }

        startLocalRot = origin->get_localRotation();
        startPos = origin->get_localPosition();
    }

    PlayerTrack::track = track;
}

void PlayerTrack::OnDidPauseEvent() {
    NELogger::GetLogger().debug("PlayerTrack::OnDidPauseEvent");
    origin->set_localRotation(startLocalRot);
    origin->set_localPosition(startPos);
}

void PlayerTrack::OnDestroy() {
    NELogger::GetLogger().debug("PlayerTrack::OnDestroy");
    if (pauseController) {
        // NELogger::GetLogger().debug("Removing action didPauseEvent %p", didPauseEventAction);
        // pauseController->remove_didPauseEvent(didPauseEventAction);
    }
    instance = nullptr;
}

void PlayerTrack::Update() {
    bool paused = false;
    if (pauseController) {
        paused = pauseController->paused;
    }

    if (!paused) {
        float noteLinesDistance = spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

        std::optional<NEVector::Quaternion> rotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.rotation);
        std::optional<NEVector::Vector3> position = getPropertyNullable<NEVector::Vector3>(track, track->properties.position);

        NEVector::Quaternion worldRotationQuaternion = startRot;
        NEVector::Vector3 positionVector = startPos;
        if (rotation.has_value() || position.has_value()) {
            NEVector::Quaternion rotationOffset = rotation.value_or(NEVector::Quaternion::get_identity());
            worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
            NEVector::Vector3 positionOffset = position.value_or(NEVector::Vector3::get_zero());
            positionVector = worldRotationQuaternion * ((positionOffset * noteLinesDistance) + startPos);
        }

        worldRotationQuaternion = worldRotationQuaternion * startLocalRot;
        std::optional<NEVector::Quaternion> localRotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.localRotation);
        if (localRotation.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *localRotation;
        }


        origin->set_localRotation(worldRotationQuaternion);



        origin->set_localPosition(positionVector);

    }
}
