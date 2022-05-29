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
#include "NECaches.h"


namespace GlobalNamespace {
    class BeatmapObjectSpawnMovementData;
    class BeatmapObjectData;
    class BeatmapObjectSpawnMovementData;
    struct NoteLineLayer;
}

namespace SpawnDataHelper {

    using namespace GlobalNamespace;

    static inline const float kHalfJumpDistanceEpsilon = 0.001f;

// CoreMathUtils.CalculateHalfJumpDurationInBeats
    constexpr float CalculateHalfJumpDurationInBeats(
            float startHalfJumpDurationInBeats,
            float maxHalfJumpDistance,
            float noteJumpMovementSpeed,
            float oneBeatDuration,
            float noteJumpStartBeatOffset)
    {
        float num1 = startHalfJumpDurationInBeats;
        float num2 = noteJumpMovementSpeed * oneBeatDuration;
        float num3 = num2 * num1;
        for (maxHalfJumpDistance -= 1.0f / 1000.0f; (double) num3 > (double) maxHalfJumpDistance; num3 = num2 * num1)
            num1 /= 2.0f;
        float jumpDurationInBeats = num1 + noteJumpStartBeatOffset;
        if ((double) jumpDurationInBeats < 0.25)
            jumpDurationInBeats = 0.25f;
        return jumpDurationInBeats;
    }


    constexpr float OneBeatDuration(float bpm)  {
        return (double) bpm <= 0.0 ? 0.0f : 60.0f / bpm;
    }


    constexpr float GetJumpDuration(BeatmapObjectSpawnController::InitData* initData, BeatmapObjectSpawnMovementData *movementData, std::optional<float> inputNjs, std::optional<float> inputOffset) {

        /*if (_initData.noteJumpValueType != BeatmapObjectSpawnMovementData.NoteJumpValueType.BeatOffset)
        {
            return _movementData.jumpDuration;
        }*/

        float oneBeatDuration = OneBeatDuration(initData->beatsPerMinute);
        float halfJumpDurationInBeats = CalculateHalfJumpDurationInBeats(movementData->startHalfJumpDurationInBeats,
                                                                         movementData->maxHalfJumpDistance,
                                                                         inputNjs.value_or(movementData->noteJumpMovementSpeed),
                                                                         oneBeatDuration,
                                                                         inputOffset.value_or(movementData->noteJumpStartBeatOffset));

        return oneBeatDuration * halfJumpDurationInBeats * 2.0f;

    }

void GetNoteJumpValues(GlobalNamespace::BeatmapObjectSpawnController::InitData *initData,
                       GlobalNamespace::BeatmapObjectSpawnMovementData* spawnMovementData,
                       std::optional<float> const inputNoteJumpMovementSpeed,
                       std::optional<float> const inputNoteJumpStartBeatOffset,
                       float &localJumpDuration, float &localJumpDistance,
                       NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos,
                       NEVector::Vector3 &localJumpEndPos);

float LineYPosForLineLayer(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> height,
                           GlobalNamespace::NoteLineLayer noteLineLayer);

inline NEVector::Vector3
GetNoteOffset(std::optional<float> startRow, std::optional<float> startHeight, int noteLineIndex,
              GlobalNamespace::NoteLineLayer noteLineLayer,
              GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData)
{
    using namespace GlobalNamespace;

    float distance = (-(spawnMovementData->noteLinesCount - 1.0f) * 0.5f) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2.0f : 0.0f);
    float lineIndex = startRow.value_or(noteLineIndex);
    distance = (distance + lineIndex) * NECaches::get_noteLinesDistanceFast();

    return (NEVector::Vector3(spawnMovementData->rightVec) * distance) + NEVector::Vector3(0, LineYPosForLineLayer(
            spawnMovementData, startHeight, noteLineLayer), 0);
}

inline UnityEngine::Vector3 GetObstacleOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, int localLineIndex, GlobalNamespace::NoteLineLayer noteLineLayer, std::optional<float> startX, std::optional<float> startY)
{
    UnityEngine::Vector3 result = GetNoteOffset(startX, startY, localLineIndex, noteLineLayer, spawnMovementData);
    result.y -= 0.15f;
    return result;
}

}