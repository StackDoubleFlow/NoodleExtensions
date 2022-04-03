#include "NELogger.h"
#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "tracks/shared/Vector.h"

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

//float SpawnDataHelperF::GetSpawnAheadTime(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNjs, std::optional<float> inputOffset) {
//    return spawnMovementData->moveDuration + (GetJumpDuration(inputNjs, inputOffset) * 0.5f);
//}



void SpawnDataHelper::GetNoteJumpValues(BeatmapObjectSpawnController::InitData *initData,
                                        BeatmapObjectSpawnMovementData *spawnMovementData,
                                        std::optional<float> const inputNoteJumpMovementSpeed,
                                        std::optional<float> const inputNoteJumpStartBeatOffset,
                                        float &localJumpDuration, float &localJumpDistance,
                                        NEVector::Vector3 &localMoveStartPos, NEVector::Vector3 &localMoveEndPos,
                                        NEVector::Vector3 &localJumpEndPos) {
    float localNoteJumpMovementSpeed = inputNoteJumpMovementSpeed.value_or(spawnMovementData->noteJumpMovementSpeed);

    localJumpDuration = GetJumpDuration(initData, spawnMovementData, localNoteJumpMovementSpeed, inputNoteJumpStartBeatOffset);
    localJumpDistance = localNoteJumpMovementSpeed * localJumpDuration;
    NEVector::Vector3 const spawnMovementDataForwardVec(spawnMovementData->forwardVec);
    NEVector::Vector3 const spawnMovementDataCenterPos(spawnMovementData->centerPos);
    localMoveStartPos = spawnMovementDataCenterPos + (spawnMovementDataForwardVec * (spawnMovementData->moveDistance + (localJumpDistance * 0.5f)));
    localMoveEndPos =   spawnMovementDataCenterPos + (spawnMovementDataForwardVec * localJumpDistance * 0.5f);
    localJumpEndPos =   spawnMovementDataCenterPos - (spawnMovementDataForwardVec * localJumpDistance * 0.5f);
}

constexpr float Orig_LineYPosForLineLayer(GlobalNamespace::NoteLineLayer lineLayer)
{
    if (lineLayer == GlobalNamespace::NoteLineLayer::Base)
    {
        return 0.25f;
    }
    if (lineLayer == GlobalNamespace::NoteLineLayer::Upper)
    {
        return 0.85f;
    }
    return 1.45f;
}

float
SpawnDataHelper::LineYPosForLineLayer(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> height,
                                      NoteLineLayer noteLineLayer) {
    if (height) {
        return  0.25f
                + (height.value() * spawnMovementData->get_noteLinesDistance()); // offset by 0.25
    }

    return Orig_LineYPosForLineLayer(noteLineLayer);
}
