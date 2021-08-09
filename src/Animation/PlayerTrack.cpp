#include "Animation/PlayerTrack.h"
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
    startPos = UnityEngine::Vector3::get_zero();
    startRot = UnityEngine::Quaternion::get_identity();
    startLocalRot = UnityEngine::Quaternion::get_identity();
    startScale = UnityEngine::Vector3::get_one();
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
        if (!pauseController) {
            didPauseEventAction = il2cpp_utils::MakeAction<Action*>(PlayerTrack::OnDidPauseEvent);
            pauseController->add_didPauseEvent(didPauseEventAction);
        }

        startLocalRot = origin->get_localRotation();
        startPos = origin->get_localPosition();
    }

    PlayerTrack::track = track;
}

void PlayerTrack::OnDidPauseEvent() {
    origin->set_localRotation(startLocalRot);
    origin->set_localPosition(startPos);
}

void PlayerTrack::OnDestroy() {
    NELogger::GetLogger().debug("PlayerTrack::OnDestroy");
    instance = nullptr;
    if (pauseController) {
        pauseController->remove_didPauseEvent(didPauseEventAction);
    }
}

void PlayerTrack::Update() {
    bool paused = false;
    if (pauseController) {
        paused = pauseController->paused;
    }

    if (!paused) {
        float noteLinesDistance = spawnController->beatmapObjectSpawnMovementData->noteLinesDistance;

        std::optional<Quaternion> rotation = track->properties.rotation.value.has_value() ?
            std::optional{ track->properties.rotation.value->quaternion } : std::nullopt;
        std::optional<Vector3> position = track->properties.position.value.has_value() ?
            std::optional{ track->properties.position.value->vector3 } : std::nullopt;

        Quaternion worldRotationQuaternion = startRot;
        Vector3 positionVector = worldRotationQuaternion * (startPos * noteLinesDistance);
        if (rotation.has_value() || position.has_value()) {
            Quaternion rotationOffset = rotation.value_or(Quaternion::get_identity());
            worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
            Vector3 positionOffset = position.value_or(Vector3::get_zero());
            positionVector = worldRotationQuaternion * ((positionOffset + startPos) * noteLinesDistance);
        }

        worldRotationQuaternion = worldRotationQuaternion * startLocalRot;
        std::optional<Quaternion> localRotation = track->properties.localRotation.value.has_value() ?
            std::optional{ track->properties.localRotation.value->quaternion } : std::nullopt;
        if (localRotation.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *localRotation;
        }

        if (origin->get_localRotation() != worldRotationQuaternion) {
            origin->set_localRotation(worldRotationQuaternion);
        }

        if (origin->get_localPosition() != positionVector) {
            origin->set_localPosition(positionVector);
        }
    }
}
