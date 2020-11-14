#pragma once

#include <optional>

namespace UnityEngine {
    struct Vector3;
}

namespace GlobalNamespace {
    class BeatmapObjectSpawnMovementData;
    class BeatmapObjectData;
}

namespace NoodleExtensions::SpawnDataHelper {

UnityEngine::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, GlobalNamespace::BeatmapObjectData *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight);

void GetNoteJumpValues(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration, 
                       float &localJumpDistance, UnityEngine::Vector3 &localMoveStartPos, UnityEngine::Vector3 &localMoveEndPos, UnityEngine::Vector3 &localJumpEndPos);

}