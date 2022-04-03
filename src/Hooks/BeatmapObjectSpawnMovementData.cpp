#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
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

#include <cmath>

using namespace GlobalNamespace;
using namespace NEVector;

BeatmapObjectSpawnController * beatmapObjectSpawnController;

MAKE_HOOK_MATCH(BeatmapObjectSpawnController_Start, &BeatmapObjectSpawnController::Start,
                void,
                BeatmapObjectSpawnController *self) {
    beatmapObjectSpawnController = self;
    BeatmapObjectSpawnController_Start(self);
}

MAKE_HOOK_MATCH(GetObstacleSpawnData, &BeatmapObjectSpawnMovementData::GetObstacleSpawnData,
                BeatmapObjectSpawnMovementData::ObstacleSpawnData,
                BeatmapObjectSpawnMovementData *self, ObstacleData *normalObstacleData) {
    if (!Hooks::isNoodleHookEnabled())
        return GetObstacleSpawnData(self, normalObstacleData);

    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(normalObstacleData);
    BeatmapObjectSpawnMovementData::ObstacleSpawnData result =
        GetObstacleSpawnData(self, obstacleData);

    // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
    if (!obstacleData->customData->value) {
        return result;
    }
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    std::optional<float> startX = ad.objectData.startX;
    std::optional<float> startY = ad.objectData.startY;

    std::optional<float> const& njs = ad.objectData.noteJumpMovementSpeed;
    std::optional<float> const& spawnOffset = ad.objectData.noteJumpStartBeatOffset;

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
    SpawnDataHelper::GetNoteJumpValues(beatmapObjectSpawnController->initData, self, njs, spawnOffset, jumpDuration,
                                       jumpDistance,
                                       localMoveStartPos, localMoveEndPos, localJumpEndPos);

    // original code has this line, not sure how important it is
    ////obstacleOffset.y = Mathf.Max(obstacleOffset.y, this._verticalObstaclePosY);

    Vector3 obstacleOffset =
            SpawnDataHelper::GetObstacleOffset(self, obstacleData->lineIndex, obstacleData->lineLayer, startX, startY);

    obstacleOffset.y += self->get_jumpOffsetY();

    moveStartPos = localMoveStartPos + obstacleOffset;
    moveEndPos = localMoveEndPos + obstacleOffset;
    jumpEndPos = localJumpEndPos + obstacleOffset;


    if (height.has_value()) {
        obstacleHeight = std::min(height.value() * obstacleData->height * 0.6f, self->obstacleTopPosY - obstacleOffset.y);
    } else {
        // _topObstaclePosY =/= _obstacleTopPosY
        obstacleHeight = std::min(
                obstacleData->height * 0.6f,
                self->obstacleTopPosY - obstacleOffset.y);
    }

    result = BeatmapObjectSpawnMovementData::ObstacleSpawnData(
        moveStartPos, moveEndPos, jumpEndPos, obstacleHeight, result.moveDuration, jumpDuration,
        self->get_noteLinesDistance());

    ad.noteOffset = NEVector::Vector3(self->centerPos) + obstacleOffset;
    ad.xOffset =
        ((width.value_or(obstacleData->lineIndex) / 2.0f) - 0.5f) * self->get_noteLinesDistance();

    return result;
}

MAKE_HOOK_MATCH(GetJumpingNoteSpawnData, &BeatmapObjectSpawnMovementData::GetJumpingNoteSpawnData,
                BeatmapObjectSpawnMovementData::NoteSpawnData, BeatmapObjectSpawnMovementData *self,
                NoteData *normalNoteData) {
    if (!Hooks::isNoodleHookEnabled())
        return GetJumpingNoteSpawnData(self, normalNoteData);

    BeatmapObjectSpawnMovementData::NoteSpawnData result = GetJumpingNoteSpawnData(self, normalNoteData);
    auto noteDataCast = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(normalNoteData);
    if (!noteDataCast)
        return result;

    auto noteData = *noteDataCast;
    if (!noteData->customData) {
        return result;
    }

    BeatmapObjectAssociatedData &ad = getAD(noteData->customData);

    std::optional<float> const njs = ad.objectData.noteJumpMovementSpeed;
    std::optional<float> const spawnOffset = ad.objectData.noteJumpStartBeatOffset;
    std::optional<float> const startLineLayer = ad.startNoteLineLayer;
    std::optional<float> const flipLineIndex =
            ad.flip ? std::optional{ad.flip->x} : std::nullopt;

    bool const gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

    std::optional<float> startRow = ad.objectData.startX;
    std::optional<float> startHeight = ad.objectData.startY;

    float jumpDuration = self->jumpDuration;

//    Vector3 moveStartPos = result.moveStartPos;
//    Vector3 moveEndPos = result.moveEndPos;
//    Vector3 jumpEndPos = result.jumpEndPos;
    float jumpGravity = result.jumpGravity;

    Vector3 const noteOffset = SpawnDataHelper::GetNoteOffset(
            self, noteData->lineIndex, noteData->noteLineLayer, startRow, startLineLayer.value_or(noteData->beforeJumpNoteLineLayer));


    float localJumpDistance;
    Vector3 localMoveStartPos;
    Vector3 localMoveEndPos;
    Vector3 localJumpEndPos;
    SpawnDataHelper::GetNoteJumpValues(beatmapObjectSpawnController->initData, self, njs, spawnOffset, jumpDuration,
                                       localJumpDistance, localMoveStartPos, localMoveEndPos,
                                       localJumpEndPos);
    float const localNoteJumpMovementSpeed = njs.value_or(self->noteJumpMovementSpeed);

    float const startLayerLineYPos = SpawnDataHelper::LineYPosForLineLayer(self, startHeight, noteData->beforeJumpNoteLineLayer);
    float const lineYPos = SpawnDataHelper::LineYPosForLineLayer(self, startLineLayer, noteData->beforeJumpNoteLineLayer);

    // Magic numbers below found with linear regression y=mx+b using existing HighestJumpPosYForLineLayer values
    float const highestJump = startHeight.has_value()
                        ? (0.875f * lineYPos) + 0.639583f + self->get_jumpOffsetY()
                        : self->HighestJumpPosYForLineLayer(noteData->noteLineLayer);

    std::function<float(float)> const GetJumpGravity = [&startLayerLineYPos, &gravityOverride, &highestJump, &localJumpDistance, &localNoteJumpMovementSpeed](float lineYPos) constexpr {
        return 2.0f * (highestJump - (gravityOverride ? lineYPos : startLayerLineYPos)) /
               std::pow(localJumpDistance / localNoteJumpMovementSpeed * 0.5f, 2.0f);
    };

    jumpGravity = GetJumpGravity(startLayerLineYPos);

    float const newJumpGravity = gravityOverride ? GetJumpGravity(lineYPos) : jumpGravity;

    Vector3 const jumpEndPos = localJumpEndPos + noteOffset;

    std::optional<float> offsetStartRow = flipLineIndex.has_value() ? flipLineIndex : startRow;
    std::optional<float> offsetStartHeight =
            gravityOverride ? startHeight
                            : float(startLineLayer.value_or((float) noteData->beforeJumpNoteLineLayer));

    Vector3 const noteOffset2 =
            SpawnDataHelper::GetNoteOffset(self, noteData->lineIndex, noteData->noteLineLayer, offsetStartRow, offsetStartHeight);
    Vector3 const moveStartPos = localMoveStartPos + noteOffset2;
    Vector3 const moveEndPos = localMoveEndPos + noteOffset2;

    result = BeatmapObjectSpawnMovementData::NoteSpawnData(
            moveStartPos, moveEndPos, jumpEndPos, newJumpGravity, result.moveDuration, jumpDuration);



    // DEFINITE POSITION IS WEIRD, OK?
    // fuck
    float num = jumpDuration * 0.5f;
    float startVerticalVelocity = jumpGravity * num;
    float yOffset = (startVerticalVelocity * num) - (jumpGravity * num * num * 0.5f);
    ad.noteOffset = Vector3(self->centerPos) + noteOffset + Vector3(0, yOffset, 0);


    return result;
}

void InstallBeatmapObjectSpawnMovementDataHooks(Logger &logger) {
    INSTALL_HOOK(logger, GetObstacleSpawnData);
    INSTALL_HOOK(logger, GetJumpingNoteSpawnData);
    INSTALL_HOOK(logger, BeatmapObjectSpawnController_Start)
}

NEInstallHooks(InstallBeatmapObjectSpawnMovementDataHooks);