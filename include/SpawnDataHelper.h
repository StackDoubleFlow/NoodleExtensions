#pragma once

#include <optional>

namespace NEVector {
    struct Vector3;
}

namespace GlobalNamespace {
    class BeatmapObjectSpawnMovementData;
    class BeatmapObjectData;
}

namespace SpawnDataHelper {

NEVector::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, GlobalNamespace::BeatmapObjectData *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight);

void GetNoteJumpValues(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration, 
                       float &localJumpDistance, NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos, NEVector::Vector3 &localJumpEndPos);

float LineYPosForLineLayer(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, GlobalNamespace::BeatmapObjectData *beatmapObjectData, std::optional<float> height);

}