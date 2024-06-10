#include "Animation/PlayerTrack.h"
#include "Animation/AnimationHelper.h"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
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
  PlayerTrack* targetInstance;

  switch (object) {
  case PlayerTrackObject::Root:
    targetInstance = entirePlayerInstance;
  case PlayerTrackObject::Head:
    targetInstance = HMDInstance;
  case PlayerTrackObject::LeftHand:
    targetInstance = leftHandInstance;
  case PlayerTrackObject::RightHand:
    targetInstance = rightHandInstance;
  default:
    targetInstance = nullptr;
  }

  auto instanceTrack = targetInstance ? targetInstance->track : nullptr;
  GameObject* noodleObject = targetInstance ? targetInstance->origin->gameObject : nullptr;

  if (instanceTrack && targetInstance) {
    instanceTrack->RemoveGameObject(targetInstance->get_gameObject());
  }

  // Init
  if (!targetInstance) {
    UnityEngine::Transform* player;
    auto playerTransforms = Resources::FindObjectsOfTypeAll<PlayerTransforms*>()->FirstOrDefault();
    if(!playerTransforms) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::ERR>("PlayerTransforms not found");
      return;
    }
    switch (object) {
    case PlayerTrackObject::Head:
      player = GameObject::Find("VRGameCore/MainCamera")->get_transform();
      break;
    case PlayerTrackObject::LeftHand:
      player = playerTransforms->_leftHandTransform;
      break;
    case PlayerTrackObject::RightHand:
      player = playerTransforms->_rightHandTransform;
      break;
    case PlayerTrackObject::Root:
    default:
      player = GameObject::Find("LocalPlayerGameCore")->get_transform();
      break;
    }

    noodleObject = GameObject::New_ctor("NoodlePlayerTrack");

    targetInstance = noodleObject->AddComponent<PlayerTrack*>();

    // Transform hierarchy manipulation: PLAYER PARENT -> NOODLE -> PLAYER
    targetInstance->origin = noodleObject->transform;
    targetInstance->origin->SetParent(player->parent, true);
    player->SetParent(targetInstance->origin, true);

    targetInstance->startLocalRot = targetInstance->origin->get_localRotation();
    targetInstance->startPos = targetInstance->origin->get_localPosition();

    targetInstance->pauseController = Object::FindObjectOfType<PauseController*>();

    if (targetInstance->pauseController) {
      std::function<void()> pause = [targetInstance]() { targetInstance->OnDidPauseEvent(); };
      std::function<void()> resume = [targetInstance]() { targetInstance->OnDidResumeEvent(); };
      didPauseEventAction = custom_types::MakeDelegate<Action*>(pause);
      targetInstance->pauseController->add_didPauseEvent(didPauseEventAction);
      didResumeEventAction = custom_types::MakeDelegate<Action*>(resume);
      targetInstance->pauseController->add_didResumeEvent(didResumeEventAction);
    }

    auto* pauseMenuManager = targetInstance->pauseController
                                 ? targetInstance->pauseController->_pauseMenuManager.ptr()
                                 : NECaches::GameplayCoreContainer->TryResolve<PauseMenuManager*>();
    auto multiPauseMenuManager =
        NECaches::GameplayCoreContainer->TryResolve<MultiplayerLocalActivePlayerInGameMenuController*>();
    if (pauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting transform to pause menu");
      pauseMenuManager->get_transform()->SetParent(targetInstance->origin, false);
    }

    if (multiPauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting multi transform to pause menu");
      multiPauseMenuManager->get_transform()->SetParent(targetInstance->origin, false);
    }

    switch (object) {
    case PlayerTrackObject::Root:
      entirePlayerInstance = targetInstance;
      break;
    case PlayerTrackObject::Head:
      HMDInstance = targetInstance;
      break;
    case PlayerTrackObject::LeftHand:
      leftHandInstance = targetInstance;
      break;
    case PlayerTrackObject::RightHand:
      rightHandInstance = targetInstance;
      break;
    default:
      break;
    }
  }

  targetInstance->set_enabled(track->v2);
  targetInstance->track = track;

  if (track->v2) {
    targetInstance->Update();
  } else {
    targetInstance->trackController =
        Tracks::GameObjectTrackController::HandleTrackData(noodleObject, { track }, 0.6, track->v2, true)
            .value_or(nullptr);
    targetInstance->trackController->UpdateData(true);
  }

  if (targetInstance && targetInstance->track) {
    targetInstance->track->AddGameObject(targetInstance->get_gameObject());
  }
}

void PlayerTrack::OnDidPauseEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidPauseEvent");
  IL2CPP_CATCH_HANDLER(
      if (selfInstance) { selfInstance->set_enabled(false); }

      if (trackController) { trackController->set_enabled(false); })
}

void PlayerTrack::OnDidResumeEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidResumeEvent");
  IL2CPP_CATCH_HANDLER(
      if (selfInstance) { selfInstance->set_enabled(track->v2); }

      if (trackController) { trackController->set_enabled(true); })
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