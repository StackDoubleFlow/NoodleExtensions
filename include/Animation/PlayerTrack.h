#pragma once
#include "tracks/shared/Animation/Track.h"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "custom-types/shared/macros.hpp"
#include "NELogger.h"

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "GlobalNamespace/PauseController.hpp"
#include "tracks/shared/Animation/GameObjectTrackController.hpp"
#include <unordered_map>

enum struct PlayerTrackObject;

DECLARE_CLASS_CODEGEN(TrackParenting, PlayerTrack, UnityEngine::MonoBehaviour) {
  DECLARE_CTOR(ctor);
  DECLARE_INSTANCE_METHOD(void, Update);
  DECLARE_INSTANCE_METHOD(void, OnDestroy);

  void UpdateDataOld();

  uint64_t lastCheckedTime = 0;

public:
  static PlayerTrack* Create(PlayerTrackObject object);
  static void AssignTrack(TrackW track, PlayerTrackObject object);

private:
  static std::unordered_map<PlayerTrackObject, SafePtrUnity<PlayerTrack>> playerTracks;
  PlayerTrackObject trackObject;

  Tracks::GameObjectTrackController* trackController;
  TrackW track;
  NEVector::Vector3 startPos;
  NEVector::Quaternion startRot;
  NEVector::Quaternion startLocalRot;
  NEVector::Vector3 startScale;
  UnityEngine::Transform* origin;
  SafePtrUnity<GlobalNamespace::PauseController> pauseController;

  void OnDidPauseEvent();
  void OnDidResumeEvent();
};