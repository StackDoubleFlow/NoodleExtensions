#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

UnityEngine::Vector3 SpawnDataHelper::GetNoteOffset(BeatmapObjectSpawnMovementData *spawnMovementData, BeatmapObjectData *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight) {
    float distance = (-(spawnMovementData->noteLinesCount - 1) * 0.5) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2 : 0);
    float lineIndex = startRow.value_or(beatmapObjectData->lineIndex);
    distance = (distance + lineIndex) * spawnMovementData->noteLinesDistance;

    return (spawnMovementData->rightVec * distance) + UnityEngine::Vector3(0, LineYPosForLineLayer(spawnMovementData, beatmapObjectData, startHeight), 0);
}

void SpawnDataHelper::GetNoteJumpValues(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration, 
                       float &localJumpDistance, UnityEngine::Vector3 &localMoveStartPos, UnityEngine::Vector3 &localMoveEndPos, UnityEngine::Vector3 &localJumpEndPos) {
    float localNoteJumpMovementSpeed = inputNoteJumpMovementSpeed.value_or(spawnMovementData->noteJumpMovementSpeed);
    float localNoteJumpStartBeatOffset = inputNoteJumpStartBeatOffset.value_or(spawnMovementData->noteJumpStartBeatOffset);
    float num = 60 / spawnMovementData->startBpm;
    float num2 = spawnMovementData->startHalfJumpDurationInBeats;
    while (localNoteJumpMovementSpeed * num * num2 > spawnMovementData->maxHalfJumpDistance) {
        num2 /= 2;
    }

    num2 += localNoteJumpStartBeatOffset;
    if (num2 < 1) {
        num2 = 1;
    }

    localJumpDuration = num * num2 * 2;
    localJumpDistance = localNoteJumpMovementSpeed * localJumpDuration;
    localMoveStartPos = spawnMovementData->centerPos + (spawnMovementData->forwardVec * (spawnMovementData->moveDistance + (localJumpDistance * 0.5)));
    localMoveEndPos =   spawnMovementData->centerPos + (spawnMovementData->forwardVec * localJumpDistance * 0.5);
    localJumpEndPos =   spawnMovementData->centerPos - (spawnMovementData->forwardVec * localJumpDistance * 0.5);
}

float SpawnDataHelper::LineYPosForLineLayer(BeatmapObjectSpawnMovementData *spawnMovementData, BeatmapObjectData *beatmapObjectData, std::optional<float> height) {
    float ypos = spawnMovementData->baseLinesYPos;
    if (height.has_value()) {
        ypos = (height.value() * spawnMovementData->noteLinesDistance) + spawnMovementData->baseLinesYPos;
    } else if (beatmapObjectData->klass == classof(CustomJSONData::CustomNoteData *)) { // This check is untested
        auto noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
        ypos = spawnMovementData->LineYPosForLineLayer(noteData->noteLineLayer);
    }

    return ypos;
}
