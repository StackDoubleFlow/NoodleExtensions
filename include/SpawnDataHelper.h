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
}

namespace SpawnDataHelper {

//    static inline const float kHalfJumpDistanceEpsilon = 0.001f;
//
//    // CoreMathUtils.CalculateHalfJumpDurationInBeats
//    static float CalculateHalfJumpDurationInBeats(
//            float startHalfJumpDurationInBeats,
//            float maxHalfJumpDistance,
//            float noteJumpMovementSpeed,
//            float oneBeatDuration,
//            float noteJumpStartBeatOffset)
//    {
//        float num1 = startHalfJumpDurationInBeats;
//        float num2 = noteJumpMovementSpeed * oneBeatDuration;
//        float num3 = num2 * num1;
//        for (maxHalfJumpDistance -= 1.0f / 1000.0f; (double) num3 > (double) maxHalfJumpDistance; num3 = num2 * num1)
//            num1 /= 2.0f;
//        float jumpDurationInBeats = num1 + noteJumpStartBeatOffset;
//        if ((double) jumpDurationInBeats < 0.25)
//            jumpDurationInBeats = 0.25f;
//        return jumpDurationInBeats;
//    }
//
//
//    float OneBeatDuration(float bpm)  {
//        return (double) bpm <= 0.0 ? 0.0f : 60.0f / bpm;
//    }
//
//
//    float GetJumpDuration(GlobalNamespace::BeatmapObjectSpawnController::InitData* initData, BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNjs, std::optional<float> inputOffset);
//
//    float GetSpawnAheadTime(GlobalNamespace::BeatmapObjectSpawnMovementData* movementData, std::optional<float> inputNjs, std::optional<float> inputOffset);







void GetNoteJumpValues(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration,
                       float &localJumpDistance, NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos, NEVector::Vector3 &localJumpEndPos);

float LineYPosForLineLayer(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, GlobalNamespace::BeatmapObjectData *beatmapObjectData, std::optional<float> height);

inline float LineYPosForLineLayer(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, GlobalNamespace::BeatmapObjectData *beatmapObjectData, float height) {
    return LineYPosForLineLayer(spawnMovementData, beatmapObjectData, std::make_optional(height));
}

    inline NEVector::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, auto *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight)
    {
        using namespace GlobalNamespace;

        float distance = (-(spawnMovementData->noteLinesCount - 1.0f) * 0.5f) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2.0f : 0.0f);
        float lineIndex = startRow.value_or(beatmapObjectData->lineIndex);
        distance = (distance + lineIndex) * spawnMovementData->get_noteLinesDistance();

        return (NEVector::Vector3(spawnMovementData->rightVec) * distance) + NEVector::Vector3(0, LineYPosForLineLayer(spawnMovementData, beatmapObjectData, startHeight), 0);
    }

    inline UnityEngine::Vector3 GetObstacleOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, auto *beatmapObjectData, std::optional<float> startX, std::optional<float> startY)
    {
        UnityEngine::Vector3 result = GetNoteOffset(spawnMovementData, beatmapObjectData, startX, startY);
        result.y -= 0.15f;
        return result;
    }
}