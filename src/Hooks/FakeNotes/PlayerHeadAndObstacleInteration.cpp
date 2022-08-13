#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"


#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/Transform.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "tracks/shared/Vector.h"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles,
                &PlayerHeadAndObstacleInteraction::RefreshIntersectingObstacles,
                void, PlayerHeadAndObstacleInteraction *self, Vector3 worldPos) {
    PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles(
        self, worldPos);

    if (!Hooks::isNoodleHookEnabled())
        return;

    // Replaced in transpile
    ArrayW<ObstacleController *> vObstacleControllers(self->intersectingObstacles->get_Count());
    self->intersectingObstacles->CopyTo(vObstacleControllers);

    int size = vObstacleControllers.size();

    for (int i = 0; i < size; i++) {
        if (i >= size) break;

        auto *obstacleController = vObstacleControllers[i];
        if (NEVector::Vector3(obstacleController->bounds.get_size()) == NEVector::Vector3::zero()) {
            self->intersectingObstacles->Remove(obstacleController);
        }
    }
}

void InstallPlayerHeadAndObstacleInterationHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles);
}

NEInstallHooks(InstallPlayerHeadAndObstacleInterationHooks);