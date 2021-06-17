#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleData.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;


MAKE_HOOK_OFFSETLESS(ObstacleData_MirrorLineIndex, void, ObstacleData *self, int lineCount) {
    // TODO
    return ObstacleData_MirrorLineIndex(self, lineCount);
}

void InstallObstacleDataHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, ObstacleData_MirrorLineIndex, il2cpp_utils::FindMethodUnsafe("", "ObstacleData", "MirrorLineIndex", 1));
}

NEInstallHooks(InstallObstacleDataHooks);