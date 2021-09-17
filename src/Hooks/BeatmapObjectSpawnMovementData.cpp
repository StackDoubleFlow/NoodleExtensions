#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectType.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/ValueType.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "SpawnDataHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomBeatmapSaveData.h"

#include <cmath>

using namespace GlobalNamespace;
using namespace NEVector;

MAKE_HOOK_MATCH(GetObstacleSpawnData, &BeatmapObjectSpawnMovementData::GetObstacleSpawnData,
                BeatmapObjectSpawnMovementData::ObstacleSpawnData,
                BeatmapObjectSpawnMovementData *self, ObstacleData *normalObstacleData) {
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(normalObstacleData);
    BeatmapObjectSpawnMovementData::ObstacleSpawnData result =
        GetObstacleSpawnData(self, obstacleData);

    // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
    if (!obstacleData->customData->value) {
        return result;
    }
    rapidjson::Value &customData = *obstacleData->customData->value;
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    std::optional<Vector2> &position = ad.objectData.position;
    std::optional<float> startX = position ? std::optional{position->x} : std::nullopt;
    std::optional<float> startY = position ? std::optional{position->y} : std::nullopt;

    std::optional<float> &njs = ad.objectData.noteJumpMovementSpeed;
    std::optional<float> &spawnOffset = ad.objectData.noteJumpStartBeatOffset;

    auto &scale = ad.objectData.scale;
    std::optional<float> width = scale && scale->at(0) ? scale->at(0) : std::nullopt;
    std::optional<float> height = scale && scale->at(1) ? scale->at(1) : std::nullopt;

    Vector3 moveStartPos = result.moveStartPos;
    Vector3 moveEndPos = result.moveEndPos;
    Vector3 jumpEndPos = result.jumpEndPos;
    float obstacleHeight = result.obstacleHeight;

    float jumpDuration;
    float jumpDistance;
    Vector3 localMoveStartPos;
    Vector3 localMoveEndPos;
    Vector3 localJumpEndPos;
    SpawnDataHelper::GetNoteJumpValues(self, njs, spawnOffset, jumpDuration, jumpDistance,
                                       localMoveStartPos, localMoveEndPos, localJumpEndPos);

    std::optional<Vector3> finalNoteOffset = std::nullopt;

    if (startX.has_value() || startY.has_value() || njs.has_value() || spawnOffset.has_value()) {
        Vector3 noteOffset =
            SpawnDataHelper::GetNoteOffset(self, obstacleData, startX, std::nullopt);
        noteOffset.y = startY.has_value()
                           ? self->verticalObstaclePosY + (startY.value() * self->noteLinesDistance)
                           : ((obstacleData->obstacleType == ObstacleType::Top)
                                  ? (self->topObstaclePosY + self->jumpOffsetY)
                                  : self->verticalObstaclePosY);

        finalNoteOffset = noteOffset;

        moveStartPos = localMoveStartPos + noteOffset;
        moveEndPos = localMoveEndPos + noteOffset;
        jumpEndPos = localJumpEndPos + noteOffset;
    }

    if (height.has_value()) {
        obstacleHeight = height.value() * self->noteLinesDistance;
    }

    result = BeatmapObjectSpawnMovementData::ObstacleSpawnData(
        moveStartPos, moveEndPos, jumpEndPos, obstacleHeight, result.moveDuration, jumpDuration,
        self->noteLinesDistance);

    if (!finalNoteOffset.has_value()) {
        Vector3 noteOffset =
            SpawnDataHelper::GetNoteOffset(self, obstacleData, startX, std::nullopt);
        noteOffset.y = (obstacleData->get_obstacleType() == ObstacleType::Top)
                           ? (self->topObstaclePosY + self->jumpOffsetY)
                           : self->verticalObstaclePosY;
        finalNoteOffset = noteOffset;
    }

    ad.noteOffset = self->centerPos + *finalNoteOffset;
    ad.xOffset =
        ((width.value_or(obstacleData->lineIndex) / 2.0f) - 0.5f) * self->noteLinesDistance;

    return result;
}

MAKE_HOOK_MATCH(GetJumpingNoteSpawnData, &BeatmapObjectSpawnMovementData::GetJumpingNoteSpawnData,
                BeatmapObjectSpawnMovementData::NoteSpawnData, BeatmapObjectSpawnMovementData *self,
                NoteData *normalNoteData) {
    BeatmapObjectSpawnMovementData::NoteSpawnData result = GetJumpingNoteSpawnData(self, normalNoteData);
    auto noteDataCast = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(normalNoteData);
    if (!noteDataCast)
        return result;

    auto noteData = *noteDataCast;
    if (!noteData->customData->value) {
        return result;
    }

    BeatmapObjectAssociatedData &ad = getAD(noteData->customData);

    std::optional<Vector2> &position = ad.objectData.position;
    std::optional<float> &njs = ad.objectData.noteJumpMovementSpeed;
    std::optional<float> &spawnOffset = ad.objectData.noteJumpStartBeatOffset;
    std::optional<float> startLineLayer = static_cast<float>(ad.startNoteLineLayer);
    std::optional<float> flipLineIndex =
            ad.objectData.flip ? std::optional{ad.objectData.flip->x} : std::nullopt;

    bool gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

    std::optional<float> startRow = position ? std::optional{position->x} : std::nullopt;
    std::optional<float> startHeight = position ? std::optional{position->y} : std::nullopt;

    float jumpDuration = self->jumpDuration;

    Vector3 moveStartPos = result.moveStartPos;
    Vector3 moveEndPos = result.moveEndPos;
    Vector3 jumpEndPos = result.jumpEndPos;
    float jumpGravity = result.jumpGravity;

    Vector3 noteOffset = SpawnDataHelper::GetNoteOffset(
            self, noteData, startRow, startLineLayer.value_or(noteData->beforeJumpNoteLineLayer));

    if (position.has_value() || flipLineIndex.has_value() || njs.has_value() ||
        spawnOffset.has_value() || startLineLayer.has_value() || gravityOverride) {
        float localJumpDuration;
        float localJumpDistance;
        Vector3 localMoveStartPos;
        Vector3 localMoveEndPos;
        Vector3 localJumpEndPos;
        SpawnDataHelper::GetNoteJumpValues(self, njs, spawnOffset, localJumpDuration,
                                           localJumpDistance, localMoveStartPos, localMoveEndPos,
                                           localJumpEndPos);
        jumpDuration = localJumpDuration;

        float localNoteJumpMovementSpeed = njs.value_or(self->noteJumpMovementSpeed);

        float startLayerLineYPos = SpawnDataHelper::LineYPosForLineLayer(
                self, noteData, startLineLayer.value_or(noteData->beforeJumpNoteLineLayer));
        float lineYPos = SpawnDataHelper::LineYPosForLineLayer(self, noteData, startHeight);

        float highestJump = startHeight.has_value()
                            ? (0.875f * lineYPos) + 0.639583f + self->jumpOffsetY
                            : self->HighestJumpPosYForLineLayer(noteData->noteLineLayer);
        jumpGravity = 2.0f * (highestJump - (gravityOverride ? lineYPos : startLayerLineYPos)) /
                      std::pow(localJumpDistance / localNoteJumpMovementSpeed * 0.5f, 2.0f);

        jumpEndPos = localJumpEndPos + noteOffset;

        std::optional<float> offsetStartRow = flipLineIndex.has_value() ? flipLineIndex : startRow;
        std::optional<float> offsetStartHeight =
                gravityOverride ? startHeight
                                : (startLineLayer.value_or((float) noteData->beforeJumpNoteLineLayer.value));

        Vector3 noteOffset2 =
                SpawnDataHelper::GetNoteOffset(self, noteData, offsetStartRow, offsetStartHeight);
        moveStartPos = localMoveStartPos + noteOffset2;
        moveEndPos = localMoveEndPos + noteOffset2;

        result = BeatmapObjectSpawnMovementData::NoteSpawnData(
                moveStartPos, moveEndPos, jumpEndPos, jumpGravity, result.moveDuration, jumpDuration);
    }

    float startVerticalVelocity = jumpGravity * jumpDuration * 0.5f;
    float num = jumpDuration * 0.5f;
    float yOffset = (startVerticalVelocity * num) - (jumpGravity * num * num * 0.5f);
    getAD(noteData->customData).noteOffset = self->centerPos + noteOffset + Vector3(0, yOffset, 0);


    return result;
}

void InstallBeatmapObjectSpawnMovementDataHooks(Logger &logger) {
    INSTALL_HOOK(logger, GetObstacleSpawnData);
    INSTALL_HOOK(logger, GetJumpingNoteSpawnData);
}

NEInstallHooks(InstallBeatmapObjectSpawnMovementDataHooks);