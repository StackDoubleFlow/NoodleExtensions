#include "Animation/ParentObject.h"

#include <utility>
#include "Animation/AnimationHelper.h"
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

    std::optional<NEVector::Quaternion> rotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.rotation);
    std::optional<NEVector::Vector3> position = getPropertyNullable<NEVector::Vector3>(track, track->properties.position);

    NEVector::Quaternion worldRotationQuaternion = startRot;
    NEVector::Vector3 positionVector = worldRotationQuaternion * (startPos * noteLinesDistance);
    if (rotation.has_value() || position.has_value()) {
        NEVector::Quaternion rotationOffset = rotation.value_or(NEVector::Quaternion::identity());
        worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
        NEVector::Vector3 positionOffset = position.value_or(NEVector::Vector3::zero());
        positionVector = worldRotationQuaternion * ((positionOffset + startPos) * noteLinesDistance);
    }

    worldRotationQuaternion = worldRotationQuaternion * startLocalRot;
    std::optional<NEVector::Quaternion> localRotation = getPropertyNullable<NEVector::Quaternion>(track, track->properties.localRotation);
    if (localRotation.has_value()) {
        worldRotationQuaternion = worldRotationQuaternion * *localRotation;
    }

    Vector3 scaleVector = startScale;
    std::optional<NEVector::Vector3> scale = getPropertyNullable<NEVector::Vector3>(track, track->properties.scale);
    if (scale.has_value()) {
        scaleVector = startScale * scale.value();
    }

    origin->set_localRotation(worldRotationQuaternion);
    origin->set_localPosition(positionVector);
    origin->set_localScale(scaleVector);
}

void ParentObject::ResetTransformParent(Transform *transform) {
    transform->SetParent(nullptr, false);
}

static void logTransform(Transform* transform, int hierarchy = 0) {
    if (hierarchy != 0) {
        std::string tab = std::string(hierarchy * 4, ' ');
        NELogger::GetLogger().debug("%i%sChild: %s %i", hierarchy, tab.c_str(), to_utf8(csstrtostr(transform->get_gameObject()->get_name())).c_str(), transform->GetChildCount());
    } else {
        NELogger::GetLogger().debug("Self: %s %i", to_utf8(csstrtostr(transform->get_gameObject()->get_name())).c_str(), transform->GetChildCount());
    }
    for (int i = 0; i < transform->GetChildCount(); i++) {
        auto childTransform = transform->GetChild(i);
        logTransform(childTransform, hierarchy + 1);
    }
}

void ParentObject::AssignTrack(const std::vector<Track *> &tracks, Track *parentTrack,
                               std::optional<NEVector::Vector3> const&  startPos,
                               std::optional<NEVector::Quaternion> const& startRot,
                               std::optional<NEVector::Quaternion> const& startLocalRot,
                               std::optional<NEVector::Vector3> const& startScale, bool worldPositionStays) {
    if (std::find(tracks.begin(), tracks.end(), parentTrack) != tracks.end()) {
        NELogger::GetLogger().error("How could a track contain itself?");
        return;
    }
    GameObject *parentGameObject = GameObject::New_ctor(il2cpp_utils::newcsstr("ParentObject"));
    ParentObject *instance = parentGameObject->AddComponent<ParentObject *>();
    instance->origin = parentGameObject->get_transform();
    instance->track = parentTrack;
    instance->worldPositionStays = worldPositionStays;

    Transform *transform = instance->get_transform();
    if (startPos.has_value()) {
        instance->startPos = *startPos;
        transform->set_localPosition(
                instance->startPos * spawnController->beatmapObjectSpawnMovementData->noteLinesDistance);
    }

    if (startRot.has_value()) {
        instance->startRot = *startRot;
        instance->startLocalRot = instance->startRot;
        transform->set_localPosition(instance->startRot * NEVector::Vector3(transform->get_localPosition()));
        transform->set_localRotation(instance->startRot);
    }

    if (startLocalRot.has_value()) {
        instance->startLocalRot = instance->startRot * *startLocalRot;
        transform->set_localRotation(NEVector::Quaternion(transform->get_localRotation()) * instance->startLocalRot);
    }

    if (startScale.has_value()) {
        instance->startScale = *startScale;
        transform->set_localScale(instance->startScale);
    }

    parentTrack->AddGameObject(parentGameObject);

    for (auto &track: tracks) {

        for (auto &parentObject: ParentController::parentObjects) {
            track->gameObjectModificationEvent -= {&ParentObject::HandleGameObject, parentObject};
            parentObject->childrenTracks.erase(track);
        }

        for (auto &gameObject: track->gameObjects) {
            instance->ParentToObject(gameObject->get_transform());
        }
        instance->childrenTracks.emplace(track);
        track->gameObjectModificationEvent += {&ParentObject::HandleGameObject, instance};
    }

    ParentController::parentObjects.emplace_back(instance);
}

void ParentObject::ParentToObject(Transform *transform) {
    transform->SetParent(origin->get_transform(), worldPositionStays);
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

void ParentObject::HandleGameObject(Track *track, UnityEngine::GameObject *go, bool removed) {
    if (removed) {
        ResetTransformParent(go->get_transform());
    } else {
        ParentToObject(go->get_transform());
    }
}

ParentObject::~ParentObject() {
    for (auto& childTrack : childrenTracks) {
        childTrack->gameObjectModificationEvent -= {&ParentObject::HandleGameObject, this};
    }
    // just in case
    track->gameObjectModificationEvent -= {&ParentObject::HandleGameObject, this};
}

void ParentController::OnDestroy() {
    NELogger::GetLogger().debug("Clearing parent objects");
    parentObjects.clear();
}