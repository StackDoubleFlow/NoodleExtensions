#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "System/Collections/Generic/List_1.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern std::vector<ObstacleController*> activeObstacles;

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_Init,
                &BasicBeatmapObjectManager::Init, void,
                BasicBeatmapObjectManager *self,
                GlobalNamespace::BasicBeatmapObjectManager::InitData* initData,
                GlobalNamespace::GameNoteController::Pool* gameNotePool,
                GlobalNamespace::BombNoteController::Pool* bombNotePool,
                GlobalNamespace::ObstacleController::Pool* obstaclePool) {
    BasicBeatmapObjectManager_Init(self, initData, gameNotePool, bombNotePool, obstaclePool);
    activeObstacles.clear();
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_get_activeObstacleControllers,
                &BasicBeatmapObjectManager::get_activeObstacleControllers,
                System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*,
                BasicBeatmapObjectManager *self) {
    // TODO: Maybe not allocate each time?
    auto list = System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>::New_ctor(activeObstacles.size());
    for (auto& oc : activeObstacles) list->Add(oc);
    return list;
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_DespawnInternal,
                static_cast<void (GlobalNamespace::BasicBeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(&GlobalNamespace::BasicBeatmapObjectManager::DespawnInternal),
                void,
                BasicBeatmapObjectManager *self,
                ObstacleController* obstacleController) {
    BasicBeatmapObjectManager_DespawnInternal(self, obstacleController);
    std::erase(activeObstacles, obstacleController);
}

void InstallBasicBeatmapObjectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, BasicBeatmapObjectManager_Init);
}

NEInstallHooks(InstallBasicBeatmapObjectManagerHooks);