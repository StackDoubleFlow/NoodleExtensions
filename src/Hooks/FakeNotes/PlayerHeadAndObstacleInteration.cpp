#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"


#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/Transform.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp"
#include "tracks/shared/Vector.h"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint,
                &PlayerHeadAndObstacleInteraction::GetObstaclesContainingPoint,
                void, PlayerHeadAndObstacleInteraction *self, Vector3 worldPos,
                List<ObstacleController *> *obstacleControllers) {
    PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint(
        self, worldPos, obstacleControllers);

    // Replaced in transpile
    VList<ObstacleController *> vObstacleControllers = obstacleControllers;

    for (int i = 0; i < vObstacleControllers.size(); i++) {
        auto *obstacleController = vObstacleControllers[i];
        if (NEVector::Vector3(obstacleController->bounds.get_size()) == NEVector::Vector3::zero()) {
            (*vObstacleControllers)->RemoveAt(i);
            i--;
        }
    }
}

MAKE_HOOK_MATCH(PlayerHeadAndObstacleInteraction_get_intersectingObstacles,
                &PlayerHeadAndObstacleInteraction::get_intersectingObstacles,
                System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*,
                        PlayerHeadAndObstacleInteraction *self) {
    auto obstaclesResult = PlayerHeadAndObstacleInteraction_get_intersectingObstacles(self);
    auto obstacles = FakeNoteHelper::ObstacleFakeCheck(obstaclesResult);

    return obstacles;
}

void InstallPlayerHeadAndObstacleInterationHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint);
    INSTALL_HOOK(logger, PlayerHeadAndObstacleInteraction_get_intersectingObstacles);
}

NEInstallHooks(InstallPlayerHeadAndObstacleInterationHooks);