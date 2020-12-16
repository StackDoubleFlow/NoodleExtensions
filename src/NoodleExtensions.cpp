#include "NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectCallbackControllerHooks();
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallBeatmapDataTransformHelperHooks();
    InstallSpawnRotationProcessorHooks();
    InstallGameplayCoreInstallerHooks();
    InstallBeatmapObjectManagerHooks();
    InstallObstacleControllerHooks();
    InstallGameNoteControllerHooks();
    InstallBombNoteControllerHooks();
    InstallNoteControllerHooks();
}