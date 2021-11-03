#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "System/Collections/Generic/List_1.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

SafePtr<List<ObstacleController*>>& getActiveObstacles();

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_Init,
                &BasicBeatmapObjectManager::Init, void,
                BasicBeatmapObjectManager *self,
                GlobalNamespace::BasicBeatmapObjectManager::InitData* initData,
                GlobalNamespace::GameNoteController::Pool* gameNotePool,
                GlobalNamespace::BombNoteController::Pool* bombNotePool,
                GlobalNamespace::ObstacleController::Pool* obstaclePool) {
    BasicBeatmapObjectManager_Init(self, initData, gameNotePool, bombNotePool, obstaclePool);
    getActiveObstacles().emplace(List<ObstacleController*>::New_ctor());
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_get_activeObstacleControllers,
                &BasicBeatmapObjectManager::get_activeObstacleControllers,
                System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*,
                BasicBeatmapObjectManager *self) {
    return (List<ObstacleController*>*) getActiveObstacles();
}


MAKE_HOOK_MATCH(BasicBeatmapObjectManager_DespawnInternal,
                static_cast<void (GlobalNamespace::BasicBeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(&GlobalNamespace::BasicBeatmapObjectManager::DespawnInternal),
                void,
                BasicBeatmapObjectManager *self,
                ObstacleController* obstacleController) {
    BasicBeatmapObjectManager_DespawnInternal(self, obstacleController);
    getActiveObstacles()->Remove(obstacleController);
}

void InstallBasicBeatmapObjectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, BasicBeatmapObjectManager_Init);
    INSTALL_HOOK(logger, BasicBeatmapObjectManager_get_activeObstacleControllers)
    INSTALL_HOOK(logger, BasicBeatmapObjectManager_DespawnInternal)

}

NEInstallHooks(InstallBasicBeatmapObjectManagerHooks);