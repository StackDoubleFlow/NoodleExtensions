#include "NELogger.h"
#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "tracks/shared/Vector.h"
#include "NECaches.h"

using namespace GlobalNamespace;



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
    localMoveEndPos =   spawnMovementDataCenterPos + (spawnMovementDataForwardVec * (localJumpDistance * 0.5f));
    localJumpEndPos =   spawnMovementDataCenterPos - (spawnMovementDataForwardVec * (localJumpDistance * 0.5f));
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
                + (height.value() * NECaches::get_noteLinesDistanceFast()); // offset by 0.25
    }

    return Orig_LineYPosForLineLayer(noteLineLayer);
}
