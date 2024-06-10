#include "Animation/PlayerTrack.h"
#include "Animation/AnimationHelper.h"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "System/Action.hpp"
#include "NECaches.h"

#include "UnityEngine/zzzz__Transform_def.hpp"
#include "custom-types/shared/delegate.hpp"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace System;
using namespace Animation;

// Events.cpp
extern BeatmapObjectSpawnController* spawnController;

static Action* didPauseEventAction;
static Action* didResumeEventAction;

DEFINE_TYPE(TrackParenting, PlayerTrack);

void PlayerTrack::ctor() {
  startPos = NEVector::Vector3::zero();
  startRot = NEVector::Quaternion::identity();
  startLocalRot = NEVector::Quaternion::identity();
  startScale = NEVector::Vector3::one();
  if (!pauseController) pauseController.emplace(nullptr);
}

void PlayerTrack::AssignTrack(Track* track, PlayerTrackObject object) {
  auto instance = [&]() -> PlayerTrack* {
    switch (object) {
      case ENTIRE_PLAYER:
        return entirePlayerInstance;
      case HMD:
        return HMDInstance;
      case LEFT_HAND:
        return leftHandInstance;
      case RIGHT_HAND:
        return rightHandInstance;
      default:
        return nullptr;
    }
  }();

  auto instanceTrack = instance ? instance->track : nullptr;

  if (instanceTrack && instance) {
    instanceTrack->RemoveGameObject(instance->get_gameObject());
  }

  if (!instance && !instanceTrack) {
    UnityEngine::Transform* player;
    switch (object) {
      case HMD:
        player = GameObject::Find("VRGameCore/MainCamera")->get_transform();
        break;
      case LEFT_HAND:
        player = GameObject::Find("VRGameCore/LeftHand")->get_transform();
        break;
      case RIGHT_HAND:
        player = GameObject::Find("VRGameCore/RightHand")->get_transform();
        break;
      case ENTIRE_PLAYER:
      default:
        player = GameObject::Find("LocalPlayerGameCore")->get_transform();
        break;
    }

    GameObject* noodleObject = GameObject::New_ctor("NoodlePlayerTrack");

    instance = noodleObject->AddComponent<PlayerTrack*>();
    instance->set_enabled(track->v2);
    instance->track = track;

    switch (object) {
      case ENTIRE_PLAYER:
        entirePlayerInstance = instance;
        break;
      case HMD:
        HMDInstance = instance;
        break;
      case LEFT_HAND:
        leftHandInstance = instance;
        break;
      case RIGHT_HAND:
        rightHandInstance = instance;
        break;
      default:
        break;
    }

    instance->origin = noodleObject->get_transform();
    instance->origin->SetParent(player->get_parent(), true);
    player->SetParent(instance->origin, true);

    auto* pauseMenuManager = instance->pauseController ? instance->pauseController->_pauseMenuManager.ptr()
                                             : NECaches::GameplayCoreContainer->TryResolve<PauseMenuManager*>();
    auto multiPauseMenuManager =
        NECaches::GameplayCoreContainer->TryResolve<MultiplayerLocalActivePlayerInGameMenuController*>();
    if (pauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting transform to pause menu");
      pauseMenuManager->get_transform()->SetParent(instance->origin, false);
    }

    if (multiPauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting multi transform to pause menu");
      multiPauseMenuManager->get_transform()->SetParent(instance->origin, false);
    }

    instance->startLocalRot = instance->origin->get_localRotation();
    instance->startPos = instance->origin->get_localPosition();

    instance->pauseController = Object::FindObjectOfType<PauseController*>();

    if (instance->pauseController) {
      std::function<void()> pause = [instance]() { instance->OnDidPauseEvent(); };
      std::function<void()> resume = [instance]() { instance->OnDidResumeEvent(); };
      didPauseEventAction = custom_types::MakeDelegate<Action*>(pause);
      instance->pauseController->add_didPauseEvent(didPauseEventAction);
      didResumeEventAction = custom_types::MakeDelegate<Action*>(resume);
      instance->pauseController->add_didResumeEvent(didResumeEventAction);
    }

    if (track->v2) {
      instance->Update();
    } else {
      instance->trackController = Tracks::GameObjectTrackController::HandleTrackData(noodleObject, {track}, 0.6, track->v2, true).value_or(nullptr);
      instance->trackController->UpdateData(true);
    }
  }

  if (instance && instance->track) {
    instance->track->AddGameObject(instance->get_gameObject());
  }
}

void PlayerTrack::OnDidPauseEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidPauseEvent");
  IL2CPP_CATCH_HANDLER(
    if (selfInstance) {
      selfInstance->set_enabled(false);
    }

    
    if (trackController) {
      trackController->set_enabled(false);
    }
  )
}

void PlayerTrack::OnDidResumeEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidResumeEvent");
  IL2CPP_CATCH_HANDLER(
    if (selfInstance) {
      selfInstance->set_enabled(track->v2);
    }

    if (trackController) {
      trackController->set_enabled(true);
    }
  )
}

void PlayerTrack::OnDestroy() {
  NELogger::Logger.debug("PlayerTrack::OnDestroy");
  if (pauseController) {
    // NELogger::Logger.debug("Removing action didPauseEvent %p", didPauseEventAction);
    // pauseController->remove_didPauseEvent(didPauseEventAction);
  }
  selfInstance = nullptr;
  trackController = nullptr;
  track = nullptr;
}

void PlayerTrack::UpdateDataOld() {
  float noteLinesDistance = NECaches::get_noteLinesDistanceFast();

  std::optional<NEVector::Quaternion> rotation =
      getPropertyNullable<NEVector::Quaternion>(track, track->properties.rotation);
  std::optional<NEVector::Vector3> position = getPropertyNullable<NEVector::Vector3>(track, track->properties.position);
  std::optional<NEVector::Quaternion> localRotation =
      getPropertyNullable<NEVector::Quaternion>(track, track->properties.localRotation);

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

void PlayerTrack::Update() {
  if (track && track->v2) {
    return UpdateDataOld();
  }
}