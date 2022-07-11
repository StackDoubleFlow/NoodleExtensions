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

        if (!inputNjs && !inputOffset && initData->noteJumpValueType == BeatmapObjectSpawnMovementData::NoteJumpValueType::JumpDuration)
        {
            return movementData->jumpDuration;
        }

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
                       float &jumpDuration, float &jumpDistance,
                       NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos,
                       NEVector::Vector3 &localJumpEndPos);

float LineYPosForLineLayer(float height);

    static NEVector::Vector2 Get2DNoteOffset(float lineIndex, int noteLinesCount, float lineLayer)
    {
        float distance = -(noteLinesCount - 1.0f) * 0.5f;
        return NEVector::Vector2((distance + lineIndex) * NECaches::get_noteLinesDistanceFast(), LineYPosForLineLayer(lineLayer));
    }

    NEVector::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, float lineIndex, float lineLayer)
    {
        NEVector::Vector2 coords = Get2DNoteOffset(lineIndex, spawnMovementData->noteLinesCount, lineLayer);
        return (spawnMovementData->rightVec * coords.x)
        + NEVector::Vector3(0, coords.y, 0);
    }

inline UnityEngine::Vector3 GetObstacleOffset(GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData, float lineIndex, float lineLayer)
{
    UnityEngine::Vector3 result = GetNoteOffset(spawnMovementData, lineIndex, lineLayer);
    result.y += -0.15f;
    return result;
}

    void NoteJumpGravityForLineLayer(
            GlobalNamespace::BeatmapObjectSpawnMovementData *spawnMovementData,
            float lineLayer,
            float startLineLayer,
            float jumpDistance,
            std::optional<float> njs,
    float& gravity,
            float& noGravity)
{
    float lineYPos = LineYPosForLineLayer(lineLayer);
    float startLayerLineYPos = LineYPosForLineLayer(startLineLayer);

    // HighestJumpPosYForLineLayer
    // Magic numbers below found with linear regression y=mx+b using existing HighestJumpPosYForLineLayer values
    float highestJump = (0.875f * lineYPos) + 0.639583f + spawnMovementData->get_jumpOffsetY();

    // NoteJumpGravityForLineLayer
    float num = jumpDistance / (njs.value_or(spawnMovementData->noteJumpMovementSpeed)) * 0.5f;
    num = 2 / (num * num);
    auto GetJumpGravity = [&](float gravityLineYPos) constexpr { return(highestJump - gravityLineYPos) * num; };
    gravity = GetJumpGravity(startLayerLineYPos);
    noGravity = GetJumpGravity(lineYPos);
}

}