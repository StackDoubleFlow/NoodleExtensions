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

std::unordered_map<PlayerTrackObject, SafePtrUnity<PlayerTrack>> PlayerTrack::playerTracks;

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
  auto& playerTrack = PlayerTrack::playerTracks[object];

  auto instanceTrack = playerTrack && playerTrack.isAlive() ? playerTrack->track : nullptr;
  GameObject* noodleObject = playerTrack ? playerTrack->origin->gameObject : nullptr;

  if (instanceTrack && playerTrack) {
    instanceTrack->RemoveGameObject(playerTrack->get_gameObject());
  }

  // Init
  if (!playerTrack) {
    auto playerTransforms = Resources::FindObjectsOfTypeAll<PlayerTransforms*>()->FirstOrDefault();
    if (!playerTransforms) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::ERR>("PlayerTransforms not found");
      return;
    }

    UnityEngine::Transform* target;
    switch (object) {
    case PlayerTrackObject::Head:
      target = playerTransforms->_headTransform;
      break;
    case PlayerTrackObject::LeftHand:
      target = playerTransforms->_leftHandTransform;
      break;
    case PlayerTrackObject::RightHand:
      target = playerTransforms->_rightHandTransform;
      break;
    case PlayerTrackObject::Root:
    default:
      target = playerTransforms->_originTransform->parent;
      break;
    }

    noodleObject = GameObject::New_ctor("NoodlePlayerTrack " + std::to_string((int)object));
    playerTrack = noodleObject->AddComponent<PlayerTrack*>();
    playerTrack->trackObject = object;
    playerTrack->origin = noodleObject->transform;


    // Transform hierarchy manipulation: PLAYER PARENT -> NOODLE -> PLAYER
    playerTrack->origin->SetParent(target->parent, true);
    target->SetParent(playerTrack->origin, true);

    playerTrack->startLocalRot = playerTrack->origin->get_localRotation();
    playerTrack->startPos = playerTrack->origin->get_localPosition();

    playerTrack->pauseController = Object::FindObjectOfType<PauseController*>();

    if (playerTrack->pauseController) {
      std::function<void()> pause = [playerTrack]() mutable { playerTrack->OnDidPauseEvent(); };
      std::function<void()> resume = [playerTrack]() mutable { playerTrack->OnDidResumeEvent(); };
      didPauseEventAction = custom_types::MakeDelegate<Action*>(pause);
      playerTrack->pauseController->add_didPauseEvent(didPauseEventAction);
      didResumeEventAction = custom_types::MakeDelegate<Action*>(resume);
      playerTrack->pauseController->add_didResumeEvent(didResumeEventAction);
    }

    auto* pauseMenuManager = playerTrack->pauseController
                                 ? playerTrack->pauseController->_pauseMenuManager.ptr()
                                 : NECaches::GameplayCoreContainer->TryResolve<PauseMenuManager*>();
    auto multiPauseMenuManager =
        NECaches::GameplayCoreContainer->TryResolve<MultiplayerLocalActivePlayerInGameMenuController*>();
    if (pauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting transform to pause menu");
      pauseMenuManager->get_transform()->SetParent(playerTrack->origin, false);
    }

    if (multiPauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting multi transform to pause menu");
      multiPauseMenuManager->get_transform()->SetParent(playerTrack->origin, false);
    }
  }

  // this is only used in v2
  playerTrack->set_enabled(track->v2);
  playerTrack->track = track;

  if (playerTrack && playerTrack->track) {
    playerTrack->track->AddGameObject(playerTrack->get_gameObject());
  }

  if (track->v2) {
    playerTrack->Update();
  } else {
    playerTrack->trackController =
        Tracks::GameObjectTrackController::HandleTrackData(noodleObject, { track }, 0.6, track->v2, true)
            .value_or(nullptr);
    playerTrack->trackController->UpdateData(true);
  }
}

void PlayerTrack::OnDidPauseEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidPauseEvent");
  this->set_enabled(false);

  if (trackController) {
    trackController->set_enabled(false);
  }
}

void PlayerTrack::OnDidResumeEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidResumeEvent");
  this->set_enabled(track->v2);

  if (trackController) {
    trackController->set_enabled(true);
  }
}

void PlayerTrack::OnDestroy() {
  NELogger::Logger.debug("PlayerTrack::OnDestroy");
  if (pauseController) {
    // NELogger::Logger.debug("Removing action didPauseEvent %p", didPauseEventAction);
    // pauseController->remove_didPauseEvent(didPauseEventAction);
  }
  trackController = nullptr;
  track = nullptr;
  PlayerTrack::playerTracks[this->trackObject] = nullptr;
}

// V2
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