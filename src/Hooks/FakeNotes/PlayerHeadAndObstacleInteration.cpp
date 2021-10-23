#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

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

    VList<ObstacleController *> vObstacleControllers = obstacleControllers;

    for (int i = 0; i < vObstacleControllers.size(); i++) {
        auto *obstacleController = vObstacleControllers[i];
        if (NEVector::Vector3(obstacleController->bounds.get_size()) == NEVector::Vector3::zero()) {
            (*vObstacleControllers)->RemoveAt(i);
            i--;
        }
    }
}

void InstallPlayerHeadAndObstacleInterationHooks(Logger &logger) {
    INSTALL_HOOK(
        logger, PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint);
}

NEInstallHooks(InstallPlayerHeadAndObstacleInterationHooks);