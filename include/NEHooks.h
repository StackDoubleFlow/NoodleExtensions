#pragma once

namespace NoodleExtensions {

void InstallBeatmapObjectCallbackControllerHooks();
void InstallBeatmapObjectSpawnMovementDataHooks();
void InstallBeatmapDataTransformHelperHooks();
void InstallSpawnRotationProcessorHooks();
void InstallGameplayCoreInstallerHooks();
void InstallObstacleControllerHooks();
void InstallNoteControllerHooks();

// Fake notes
void InstallNoteCutSoundEffectManagerHooks();
void InstallBeatmapObjectManagerHooks();
void InstallGameNoteControllerHooks();
void InstallBombNoteControllerHooks();

void InstallHooks();

}