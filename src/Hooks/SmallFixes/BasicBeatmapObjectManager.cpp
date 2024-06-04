#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "System/Collections/Generic/List_1.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

SafePtr<System::Collections::Generic::List_1<UnityW<ObstacleController>>>& getActiveObstacles();

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_Init, &BasicBeatmapObjectManager::Init, void, BasicBeatmapObjectManager* self,
                ::GlobalNamespace::__BasicBeatmapObjectManager__InitData* initData,
                ::GlobalNamespace::__GameNoteController__Pool* basicGameNotePool,
                ::GlobalNamespace::__GameNoteController__Pool* burstSliderHeadGameNotePool,
                ::GlobalNamespace::__BurstSliderGameNoteController__Pool* burstSliderGameNotePool,
                ::GlobalNamespace::__BombNoteController__Pool* bombNotePool,
                ::GlobalNamespace::__ObstacleController__Pool* obstaclePool,
                ::GlobalNamespace::__SliderController__Pool* sliderPools) {
  BasicBeatmapObjectManager_Init(self, initData, basicGameNotePool, burstSliderHeadGameNotePool,
                                 burstSliderGameNotePool, bombNotePool, obstaclePool, sliderPools);
  if (!Hooks::isNoodleHookEnabled()) return;

  //TODO: WHAT DOES THIS MEAN????
  //getActiveObstacles().emplace(List<ObstacleController*>::New_ctor());
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_get_activeObstacleControllers,
                &BasicBeatmapObjectManager::get_activeObstacleControllers,
                System::Collections::Generic::List_1<UnityW<GlobalNamespace::ObstacleController>>*,
                BasicBeatmapObjectManager* self) {
  if (!Hooks::isNoodleHookEnabled()) return BasicBeatmapObjectManager_get_activeObstacleControllers(self);

  return getActiveObstacles().ptr();
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_DespawnInternal,
                static_cast<void (GlobalNamespace::BasicBeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(
                    &GlobalNamespace::BasicBeatmapObjectManager::DespawnInternal),
                void, BasicBeatmapObjectManager* self, ObstacleController* obstacleController) {
  BasicBeatmapObjectManager_DespawnInternal(self, obstacleController);
  if (!Hooks::isNoodleHookEnabled()) return;

  getActiveObstacles()->Remove(obstacleController);
}

void InstallBasicBeatmapObjectManagerHooks() {
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_Init);
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_get_activeObstacleControllers)
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_DespawnInternal)
}

NEInstallHooks(InstallBasicBeatmapObjectManagerHooks);