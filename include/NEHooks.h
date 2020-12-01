#pragma once

namespace NoodleExtensions {

void InstallBeatmapObjectSpawnMovementDataHooks();
void InstallBeatmapDataTransformHelperHooks();
void InstallSpawnRotationProcessorHooks();
void InstallGameplayCoreInstallerHooks();
void InstallObstacleControllerHooks();
void InstallNoteControllerHooks();

// Fake notes
void InstallGameNoteControllerHooks();
void InstallBombNoteControllerHooks();

void InstallHooks();

}