#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "UnityEngine/Vector3.hpp"

#include "NEHooks.h"
#include "FakeNoteHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_OFFSETLESS(PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint, void, PlayerHeadAndObstacleInteraction *self, Vector3 worldPos, List<ObstacleController*> *obstacleControllers) {
    PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint(self, worldPos, obstacleControllers);

    VList<ObstacleController*> vObstacleControllers = obstacleControllers;

    for (int i = 0; i < vObstacleControllers.size(); i++) {
        auto *obstacleController = vObstacleControllers[i];
        if (obstacleController->bounds.get_size() == Vector3::get_zero()) {
            (*vObstacleControllers)->RemoveAt(i);
            i--;
        }
    }
}

void InstallPlayerHeadAndObstacleInterationHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, PlayerHeadAndObstacleInteraction_GetObstaclesContainingPoint, 
        il2cpp_utils::FindMethodUnsafe("", "PlayerHeadAndObstacleInteraction", "GetObstaclesContainingPoint", 2));
}

NEInstallHooks(InstallPlayerHeadAndObstacleInterationHooks);