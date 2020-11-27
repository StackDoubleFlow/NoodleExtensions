#pragma once

namespace NoodleExtensions {

void InstallBeatmapObjectSpawnMovementDataHooks();
void InstallBeatmapDataTransformHelperHooks();
void InstallSpawnRotationProcessorHooks();
void InstallGameplayCoreInstallerHooks();
void InstallObstacleControllerHooks();

// Fake notes
void InstallGameNoteControllerHooks();
void InstallBombNoteControllerHooks();

void InstallHooks();

}