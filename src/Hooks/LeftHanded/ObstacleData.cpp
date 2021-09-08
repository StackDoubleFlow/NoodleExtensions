#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/ObstacleData.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(ObstacleData_Mirror, &ObstacleData::Mirror,
                void, ObstacleData *self, int lineCount) {
    // TODO
    return ObstacleData_Mirror(self, lineCount);
}

void InstallObstacleDataHooks(Logger &logger) {
    INSTALL_HOOK(logger, ObstacleData_Mirror);
}

NEInstallHooks(InstallObstacleDataHooks);