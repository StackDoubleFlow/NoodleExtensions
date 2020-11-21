#include "NoodleExtensions/NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallBasicBeatmapObjectManagerHooks();
    InstallSpawnRotationProcessorHooks();
    InstallObstacleControllerHooks();
    InstallGameNoteControllerHooks();
    InstallBombNoteControllerHooks();
}