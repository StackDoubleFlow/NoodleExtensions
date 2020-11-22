#include "NoodleExtensions/NEHooks.h"

using namespace NoodleExtensions;

void NoodleExtensions::InstallHooks() {
    InstallBeatmapObjectSpawnMovementDataHooks();
    InstallSpawnRotationProcessorHooks();
    InstallObstacleControllerHooks();
    InstallGameNoteControllerHooks();
    InstallBombNoteControllerHooks();
}