#include "NoodleExtensions/NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallSpawnRotationProcessorProcessBeatmapEventDataHooks();
    InstallObstacleControllerHooks();
    InstallBasicBeatmapObjectManagerHooks();
}