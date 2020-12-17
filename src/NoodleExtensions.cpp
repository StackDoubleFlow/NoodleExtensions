#include "NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectCallbackControllerHooks();
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallBeatmapDataTransformHelperHooks();
    InstallNoteCutSoundEffectManagerHooks();
    InstallSpawnRotationProcessorHooks();
    InstallGameplayCoreInstallerHooks();
    InstallBeatmapObjectManagerHooks();
    InstallObstacleControllerHooks();
    InstallGameNoteControllerHooks();
    InstallBombNoteControllerHooks();
    InstallNoteControllerHooks();
}