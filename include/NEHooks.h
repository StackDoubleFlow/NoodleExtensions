#pragma once
#include "NELogger.h"

namespace NoodleExtensions {

void InstallStandardLevelScenesTransitionSetupDataSOHooks(Logger& logger);
void InstallBeatmapObjectCallbackControllerHooks(Logger& logger);
void InstallBeatmapObjectSpawnMovementDataHooks(Logger& logger);
void InstallBeatmapDataTransformHelperHooks(Logger& logger);
void InstallSpawnRotationProcessorHooks(Logger& logger);
void InstallGameplayCoreInstallerHooks(Logger& logger);
void InstallObstacleControllerHooks(Logger& logger);
void InstallBeatmapDataLoaderHooks(Logger& logger);
void InstallNoteControllerHooks(Logger& logger);
void InstallNoteFloorMovementHooks(Logger& logger);
void InstallNoteJumpHooks(Logger& logger);

// Fake notes
void InstallNoteCutSoundEffectManagerHooks(Logger& logger);
void InstallBeatmapObjectManagerHooks(Logger& logger);
void InstallGameNoteControllerHooks(Logger& logger);
void InstallBombNoteControllerHooks(Logger& logger);

// Temporary
void InstallClampPatches(Logger& logger);

void InstallHooks();

}