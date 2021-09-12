#include "Animation/ParentObject.h"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;

// Events.cpp
extern BeatmapObjectSpawnController *spawnController;

DEFINE_TYPE(TrackParenting, ParentObject);

void ParentObject::Update() {
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

    Vector3 scaleVector = startScale;
    std::optional<Vector3> scale = track->properties.scale.value.has_value() ?
        std::optional{ track->properties.scale.value->vector3 } : std::nullopt;
    if (scale.has_value()) {
        scaleVector = Vector3::Scale(startScale, *scale);
    }


    origin->set_localRotation(worldRotationQuaternion);



    origin->set_localPosition(positionVector);



    origin->set_localScale(scaleVector);

}

void ParentObject::ResetTransformParent(Transform *transform) {
    transform->SetParent(nullptr, false); 
}

void ParentObject::AssignTrack(std::vector<Track*> tracks, Track *parentTrack, std::optional<Vector3> startPos, 
        std::optional<Quaternion> startRot, std::optional<Quaternion> startLocalRot, std::optional<Vector3> startScale) {
    GameObject *parentGameObject = GameObject::New_ctor(il2cpp_utils::newcsstr("ParentObject"));
    ParentObject *instance = parentGameObject->AddComponent<ParentObject*>();
    instance->origin = parentGameObject->get_transform();
    instance->track = parentTrack;

    Transform *transform = instance->get_transform();
    if (startPos.has_value()) {
        instance->startPos = *startPos;
        transform->set_localPosition(*startPos * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance);
    }

    if (startRot.has_value()) {
        instance->startRot = *startRot;
        instance->startLocalRot = instance->startRot;
        transform->set_localPosition(instance->startRot * transform->get_localPosition());
        transform->set_localRotation(instance->startRot);
    }

    if (startLocalRot.has_value()) {
        instance->startLocalRot = *startLocalRot;
        transform->set_localRotation(instance->startRot * *startLocalRot);
    }

    if (startScale.has_value()) {
        instance->startScale = *startScale;
        transform->set_localScale(instance->startScale);
    }

    for (auto& parentObject : ParentController::parentObjects) {
        if (parentObject->childrenTracks.contains(parentTrack)) {
            parentObject->ParentToObject(transform);
        } else {
            ResetTransformParent(transform);
        }
    }

    for (auto& track : tracks) {
        for (auto& parentObject : ParentController::parentObjects) {
            parentObject->childrenTracks.erase(track);

            if (parentObject->track == track) {
                instance->ParentToObject(parentObject->get_transform());
            }
        }

        instance->childrenTracks.insert(track);
    }

    ParentController::parentObjects.insert(instance);
}

void ParentObject::ParentToObject(Transform *transform) {
    transform->SetParent(origin->get_transform(), false);
}

ParentObject *ParentController::GetParentObjectTrack(Track *track) {
    auto itr = std::find_if(parentObjects.begin(), parentObjects.end(), [=](auto *n) { 
        return n->childrenTracks.contains(track); 
    });

    if (itr != parentObjects.end()) {
        return *itr;
    } else {
        return nullptr;
    }
}

void ParentController::OnDestroy() {
    NELogger::GetLogger().debug("Clearing parent objects");
    parentObjects.clear();
}