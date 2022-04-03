#pragma once

#include <optional>

#include "tracks/shared/Vector.h"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "NELogger.h"
#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"


namespace GlobalNamespace {
    class BeatmapObjectSpawnMovementData;
    class BeatmapObjectData;
    class BeatmapObjectSpawnMovementData;
    struct NoteLineLayer;
}

namespace SpawnDataHelper {

void GetNoteJumpValues(GlobalNamespace::BeatmapObjectSpawnController::InitData *initData,
                       GlobalNamespace::BeatmapObjectSpawnMovementData* spawnMovementData,
                       std::optional<float> const inputNoteJumpMovementSpeed,
                       std::optional<float> const inputNoteJumpStartBeatOffset,
                       float &localJumpDuration, float &localJumpDistance,
                       NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos,
                       NEVector::Vector3 &localJumpEndPos);

float LineYPosForLineLayer(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> height,
                           GlobalNamespace::NoteLineLayer noteLineLayer);

inline NEVector::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, int localLineIndex, GlobalNamespace::NoteLineLayer noteLineLayer, std::optional<float> startRow, std::optional<float> startHeight)
{
    using namespace GlobalNamespace;

    float distance = (-(spawnMovementData->noteLinesCount - 1.0f) * 0.5f) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2.0f : 0.0f);
    float lineIndex = startRow.value_or(localLineIndex);
    distance = (distance + lineIndex) * spawnMovementData->get_noteLinesDistance();

    return (NEVector::Vector3(spawnMovementData->rightVec) * distance) + NEVector::Vector3(0, LineYPosForLineLayer(
            spawnMovementData, startHeight, noteLineLayer), 0);
}

inline UnityEngine::Vector3 GetObstacleOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, int localLineIndex, GlobalNamespace::NoteLineLayer noteLineLayer, std::optional<float> startX, std::optional<float> startY)
{
    UnityEngine::Vector3 result = GetNoteOffset(spawnMovementData, localLineIndex, noteLineLayer, startX, startY);
    result.y -= 0.15f;
    return result;
}

}