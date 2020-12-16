#include "NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectCallbackControllerHooks();
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallBeatmapDataTransformHelperHooks();
    InstallSpawnRotationProcessorHooks();
    InstallGameplayCoreInstallerHooks();
    InstallObstacleControllerHooks();
    InstallGameNoteControllerHooks();
    InstallBombNoteControllerHooks();
    InstallNoteControllerHooks();
}