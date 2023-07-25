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

void PlayerTrack::AssignTrack(Track* track) {
  if (PlayerTrack::track && PlayerTrack::instance && !track->v2) {
    PlayerTrack::track->RemoveGameObject(PlayerTrack::instance->get_gameObject());
  }
  PlayerTrack::track = track;

  if (!instance) {
    auto* player = GameObject::Find("LocalPlayerGameCore")->get_transform();
    GameObject* noodleObject = GameObject::New_ctor("NoodlePlayerTrack");
    origin = noodleObject->get_transform();
    origin->SetParent(player->get_parent(), true);
    player->SetParent(origin, true);

    instance = noodleObject->AddComponent<PlayerTrack*>();
    pauseController = Object::FindObjectOfType<PauseController*>();

    if (pauseController) {
      std::function<void()> pause = []() { PlayerTrack::OnDidPauseEvent(); };
      std::function<void()> resume = []() { PlayerTrack::OnDidResumeEvent(); };
      didPauseEventAction = custom_types::MakeDelegate<Action*>(pause);
      pauseController->add_didPauseEvent(didPauseEventAction);
      didResumeEventAction = custom_types::MakeDelegate<Action*>(resume);
      pauseController->add_didResumeEvent(didResumeEventAction);
    }

    auto* pauseMenuManager = pauseController ? pauseController->pauseMenuManager
                                             : NECaches::GameplayCoreContainer->TryResolve<PauseMenuManager*>();
    auto multiPauseMenuManager =
        NECaches::GameplayCoreContainer->TryResolve<MultiplayerLocalActivePlayerInGameMenuController*>();
    if (pauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting transform to pause menu");
      pauseMenuManager->get_transform()->SetParent(origin, false);
    }

    if (multiPauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting multi transform to pause menu");
      multiPauseMenuManager->get_transform()->SetParent(origin, false);
    }

    startLocalRot = origin->get_localRotation();
    startPos = origin->get_localPosition();
    instance->UpdateData(true);
  }

  if (PlayerTrack::track && PlayerTrack::instance && !track->v2) {
    PlayerTrack::track->AddGameObject(PlayerTrack::instance->get_gameObject());
  }
}

void PlayerTrack::OnDidPauseEvent() {
  NELogger::GetLogger().debug("PlayerTrack::OnDidPauseEvent");
  IL2CPP_CATCH_HANDLER(instance->set_enabled(false);)
}

void PlayerTrack::OnDidResumeEvent() {
  NELogger::GetLogger().debug("PlayerTrack::OnDidResumeEvent");
  IL2CPP_CATCH_HANDLER(instance->set_enabled(true);)
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
  UpdateData(false);
}

void PlayerTrack::UpdateData(bool force) {
  if (!track) return;

  if (track->v2) {
    return UpdateDataOld();
  }

  if (force) {
    lastCheckedTime = 0;
  }

  float noteLinesDistance = NECaches::get_noteLinesDistanceFast();

  auto const& properties = track->properties;
  auto const rotation = getPropertyNullableFast<NEVector::Quaternion>(track, properties.rotation, lastCheckedTime);
  auto const localRotation =
      getPropertyNullableFast<NEVector::Quaternion>(track, properties.localRotation, lastCheckedTime);
  auto const position = getPropertyNullableFast<NEVector::Vector3>(track, properties.position, lastCheckedTime);
  auto const localPosition =
      getPropertyNullableFast<NEVector::Vector3>(track, properties.localPosition, lastCheckedTime);
  auto const scale = getPropertyNullableFast<NEVector::Vector3>(track, properties.scale, lastCheckedTime);

  auto transform = origin;

  if (localRotation) {
    transform->set_localRotation(localRotation.value());
  } else if (rotation) {
    transform->set_rotation(rotation.value());
  }

  if (localPosition) {
    transform->set_localPosition(localPosition.value());
  } else if (position) {
    transform->set_position(position.value());
  }

  if (scale) {
    transform->set_localScale(scale.value());
  }

  lastCheckedTime = getCurrentTime();
}
