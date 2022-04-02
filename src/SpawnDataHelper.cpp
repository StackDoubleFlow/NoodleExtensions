#include "NELogger.h"
#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "tracks/shared/Vector.h"

using namespace GlobalNamespace;
//
//float SpawnDataHelperF::GetJumpDuration(BeatmapObjectSpawnController::InitData* initData, BeatmapObjectSpawnMovementData *movementData, std::optional<float> inputNjs, std::optional<float> inputOffset) {
//
//    /*if (_initData.noteJumpValueType != BeatmapObjectSpawnMovementData.NoteJumpValueType.BeatOffset)
//    {
//        return _movementData.jumpDuration;
//    }*/
//
//    float oneBeatDuration = OneBeatDuration(initData->beatsPerMinute);
//    float halfJumpDurationInBeats = CalculateHalfJumpDurationInBeats(movementData->startHalfJumpDurationInBeats,
//                                                                     movementData->maxHalfJumpDistance,
//                                                                     inputNjs.value_or(movementData->noteJumpMovementSpeed),
//    oneBeatDuration,
//    inputOffset.value_or(movementData->noteJumpStartBeatOffset));
//
//    return oneBeatDuration * halfJumpDurationInBeats * 2.0f;
//
//}
//
//float SpawnDataHelperF::GetSpawnAheadTime(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNjs, std::optional<float> inputOffset) {
//    return spawnMovementData->moveDuration + (GetJumpDuration(inputNjs, inputOffset) * 0.5f);
//}


void SpawnDataHelper::GetNoteJumpValues(BeatmapObjectSpawnMovementData* spawnMovementData,
                                        std::optional<float> const inputNoteJumpMovementSpeed,
                                        std::optional<float> const inputNoteJumpStartBeatOffset,

                                        float &localJumpDuration, float &localJumpDistance,
                                        NEVector::Vector3 &localMoveStartPos,
                                        NEVector::Vector3 &localMoveEndPos,
                                        NEVector::Vector3 &localJumpEndPos
                                        ) {
    float localNoteJumpMovementSpeed = inputNoteJumpMovementSpeed.value_or(spawnMovementData->noteJumpMovementSpeed);
    float localNoteJumpStartBeatOffset = inputNoteJumpStartBeatOffset.value_or(spawnMovementData->noteJumpStartBeatOffset);
    float num = 60.0f / spawnMovementData->startBpm;
    float num2 = spawnMovementData->startHalfJumpDurationInBeats;
    while (localNoteJumpMovementSpeed * num * num2 > spawnMovementData->maxHalfJumpDistance) {
        num2 /= 2.0f;
    }

    num2 += localNoteJumpStartBeatOffset;
    if (num2 < 1.0f) {
        num2 = 1.0f;
    }

    localJumpDuration = num * num2 * 2.0f;
    localJumpDistance = localNoteJumpMovementSpeed * localJumpDuration;
    NEVector::Vector3 const spawnMovementDataForwardVec(spawnMovementData->forwardVec);
    NEVector::Vector3 const spawnMovementDataCenterPos(spawnMovementData->centerPos);
    localMoveStartPos = spawnMovementDataCenterPos + (spawnMovementDataForwardVec * (spawnMovementData->moveDistance + (localJumpDistance * 0.5f)));
    localMoveEndPos =   spawnMovementDataCenterPos + (spawnMovementDataForwardVec * localJumpDistance * 0.5f);
    localJumpEndPos =   spawnMovementDataCenterPos - (spawnMovementDataForwardVec * localJumpDistance * 0.5f);
}

float SpawnDataHelper::LineYPosForLineLayer(BeatmapObjectSpawnMovementData *spawnMovementData, BeatmapObjectData *beatmapObjectData, std::optional<float> height) {
    float ypos = spawnMovementData->baseLinesYPos;
    if (height.has_value()) {
        ypos = (height.value() * spawnMovementData->noteLinesDistance) + spawnMovementData->baseLinesYPos;
    } else if (auto noteData = il2cpp_utils::try_cast<GlobalNamespace::NoteData>(beatmapObjectData)) {
        ypos = spawnMovementData->LineYPosForLineLayer(noteData.value()->noteLineLayer);
    }

    return ypos;
}
