#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/Action_1.hpp"
#include "System/Action_3.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"


using namespace GlobalNamespace;
using namespace UnityEngine;

static bool GetHiddenForType(BeatmapObjectManager* beatmapObjectManager) {
    static auto BasicBeatmapObjectManagerKlass = classof(BasicBeatmapObjectManager*);

    if (il2cpp_functions::class_is_assignable_from(BasicBeatmapObjectManagerKlass, beatmapObjectManager->klass))
    {
        return true;
    }

    return beatmapObjectManager->spawnHidden;
}

MAKE_HOOK_MATCH(BeatmapObjectManager_SpawnObstacle,
                &BeatmapObjectManager::AddSpawnedObstacleController,
                void,
                BeatmapObjectManager *self,
                GlobalNamespace::ObstacleController* obstacleController,
                ::GlobalNamespace::BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData,
                float rotation) {
    if (!Hooks::isNoodleHookEnabled())
        return BeatmapObjectManager_SpawnObstacle(self, obstacleController, obstacleSpawnData, rotation);

    if (obstacleController == nullptr)
    {
        return;
    }
    self->SetObstacleEventCallbacks(obstacleController);
    auto action = self->obstacleWasSpawnedEvent;
    if (action != nullptr)
    {
        action->Invoke(obstacleController);
    }
    auto action2 = self->obstacleWasAddedEvent;
    if (action2 != nullptr)
    {
        action2->Invoke(obstacleController->obstacleData, obstacleSpawnData, rotation);
    }
    obstacleController->ManualUpdate();
    // TRANSPILE HERE
    obstacleController->Hide(GetHiddenForType(self));
    ///
    self->allBeatmapObjects->Add(reinterpret_cast<IBeatmapObjectController *>(obstacleController));



    // POST FIX
    auto *customObstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(obstacleController->obstacleData);
    if (customObstacleData->customData) {
        BeatmapObjectAssociatedData &ad = getAD(customObstacleData->customData);
        ad.doUnhide = true;
    }

    //
}

void InstallBeatmapObjectManagerSmallFixHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, BeatmapObjectManager_SpawnObstacle);
}


NEInstallHooks(InstallBeatmapObjectManagerSmallFixHooks);