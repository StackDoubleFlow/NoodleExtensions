#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/Action_1.hpp"
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
                &BeatmapObjectManager::SpawnObstacle,
                GlobalNamespace::ObstacleController*,
                BeatmapObjectManager *self,
                GlobalNamespace::ObstacleData* obstacleData,
                GlobalNamespace::BeatmapObjectSpawnMovementData::ObstacleSpawnData obstacleSpawnData,
                float rotation) {
    auto obstacleController = self->SpawnObstacleInternal(obstacleData, obstacleSpawnData, rotation);
    if (obstacleController)
    {
        self->SetObstacleEventCallbacks(obstacleController);
        auto action = self->obstacleWasSpawnedEvent;
        if (action)
        {
            action->Invoke(obstacleController);
        }
        obstacleController->ManualUpdate();

        // TRANSPILE HERE
        obstacleController->set_hide(GetHiddenForType(self));
        ///
    }

    // POST FIX
    auto *customObstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(obstacleData);
    if (customObstacleData->customData) {
        BeatmapObjectAssociatedData &ad = getAD(customObstacleData->customData);
        ad.doUnhide = true;
    }

    //
    return obstacleController;
}

void InstallBeatmapObjectManagerSmallFixHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, BeatmapObjectManager_SpawnObstacle);
}


NEInstallHooks(InstallBeatmapObjectManagerSmallFixHooks);