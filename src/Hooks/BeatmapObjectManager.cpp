#include "tracks/shared/Animation/Track.h"
#include "tracks/shared/AssociatedData.h"
#include "Animation/ParentObject.h"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BeatmapObjectManager_Note_Despawn,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(GlobalNamespace::NoteController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Note_Despawn(self, noteController);

  auto customNoteData = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(noteController->noteData);

  if (customNoteData && customNoteData.value()->customData && customNoteData.value()->customData->value) {
    auto const& tracks = TracksAD::getAD(customNoteData.value()->customData).tracks;
    if (!tracks.empty()) {
      auto go = noteController->get_gameObject();
      for (auto& track : tracks) {
        track->RemoveGameObject(go);
      }
    }
  }
  BeatmapObjectManager_Note_Despawn(self, noteController);
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Obstacle_Despawn,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, ObstacleController* obstacleController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Obstacle_Despawn(self, obstacleController);

  auto customObstacleData =
      il2cpp_utils::try_cast<CustomJSONData::CustomObstacleData>(obstacleController->obstacleData);

  if (customObstacleData && customObstacleData.value()->customData && customObstacleData.value()->customData->value) {
    auto const& tracks = TracksAD::getAD(customObstacleData.value()->customData).tracks;
    if (!tracks.empty()) {
      auto go = obstacleController->get_gameObject();
      for (auto& track : tracks) {
        track->RemoveGameObject(go);
      }
    }
  }
  BeatmapObjectManager_Obstacle_Despawn(self, obstacleController);
}

void InstallBeatmapObjectManagerHooksTrack(Logger& logger) {
  INSTALL_HOOK(logger, BeatmapObjectManager_Note_Despawn);
  INSTALL_HOOK(logger, BeatmapObjectManager_Obstacle_Despawn);
}

NEInstallHooks(InstallBeatmapObjectManagerHooksTrack);