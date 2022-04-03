#include "Animation/PlayerTrack.h"
#include "Animation/AnimationHelper.h"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "System/Action.hpp"
#include "NECaches.h"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace System;
using namespace Animation;

// Events.cpp
extern BeatmapObjectSpawnController *spawnController;

static Action *didPauseEventAction;

DEFINE_TYPE(TrackParenting, PlayerTrack);

void PlayerTrack::ctor() {
    startPos = NEVector::Vector3::zero();
    startRot = NEVector::Quaternion::identity();
    startLocalRot = NEVector::Quaternion::identity();
    startScale = NEVector::Vector3::one();
}

void PlayerTrack::AssignTrack(Track *track) {
    if (!instance) {
        GameObject *gameObject = GameObject::Find("LocalPlayerGameCore");
        GameObject *noodleObject = GameObject::New_ctor("NoodlePlayerTrack");
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
    track = nullptr;
}

void PlayerTrack::Update() {
    bool paused = false;
    if (pauseController) {
        paused = pauseController->paused;
    }

    if (!paused) {
        float noteLinesDistance = spawnController->beatmapObjectSpawnMovementData->get_noteLinesDistance();

        std::optional<NEVector::Quaternion> rotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.rotation);
        std::optional<NEVector::Vector3> position = getPropertyNullable<NEVector::Vector3>(track, track->properties.position);
        std::optional<NEVector::Quaternion> localRotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.localRotation);

        if (NECaches::LeftHandedMode) {
            rotation = Animation::MirrorQuaternionNullable(rotation);
            localRotation = Animation::MirrorQuaternionNullable(localRotation);
            position = Animation::MirrorVectorNullable(position);
        }

        NEVector::Quaternion worldRotationQuaternion = startRot;
        NEVector::Vector3 positionVector = startPos;
        if (rotation.has_value() || position.has_value()) {
            NEVector::Quaternion rotationOffset = rotation.value_or(NEVector::Quaternion::identity());
            worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
            NEVector::Vector3 positionOffset = position.value_or(NEVector::Vector3::zero());
            positionVector = worldRotationQuaternion * ((positionOffset * noteLinesDistance) + startPos);
        }

        worldRotationQuaternion = worldRotationQuaternion * startLocalRot;
        if (localRotation.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *localRotation;
        }


        origin->set_localRotation(worldRotationQuaternion);
        origin->set_localPosition(positionVector);

    }
}
