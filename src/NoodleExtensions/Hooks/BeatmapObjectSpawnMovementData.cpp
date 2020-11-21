#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/BeatmapObjectType.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteSpawnData.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/ValueType.hpp"

#include "CustomJSONData/CustomBeatmapSaveData.h"
#include "CustomJSONData/CustomBeatmapData.h"
#include "CustomJSONData/CustomJSONDataHooks.h"
#include "NoodleExtensions/SpawnDataHelper.h"
#include "NoodleExtensions/NEHooks.h"
#include "NELogger.h"

#include <cmath>

using namespace NoodleExtensions;

struct BeatmapObjectSpawnMovementData_ObstacleSpawnData {
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    float obstacleHeight;
    float moveDuration;
    float jumpDuration;
    float noteLinesDistance;
    constexpr BeatmapObjectSpawnMovementData_ObstacleSpawnData(UnityEngine::Vector3 moveStartPos_ = {}, UnityEngine::Vector3 moveEndPos_ = {}, UnityEngine::Vector3 jumpEndPos_ = {}, float obstacleHeight_ = {}, float moveDuration_ = {}, float jumpDuration_ = {}, float noteLinesDistance_ = {}) noexcept : moveStartPos{moveStartPos_}, moveEndPos{moveEndPos_}, jumpEndPos{jumpEndPos_}, obstacleHeight{obstacleHeight_}, moveDuration{moveDuration_}, jumpDuration{jumpDuration_}, noteLinesDistance{noteLinesDistance_} {}
};

MAKE_HOOK_OFFSETLESS(GetObstacleSpawnData, BeatmapObjectSpawnMovementData_ObstacleSpawnData, BeatmapObjectSpawnMovementData *self, CustomJSONData::CustomObstacleData *obstacleData) {
    BeatmapObjectSpawnMovementData_ObstacleSpawnData result = GetObstacleSpawnData(self, obstacleData);

    static_assert(sizeof(BeatmapObjectSpawnMovementData_ObstacleSpawnData) == 0x34);

    // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
    if (!obstacleData->customData) {
        return result;
    }

    rapidjson::Value &customData = *obstacleData->customData;

    std::optional<rapidjson::Value*> position = customData.HasMember("_position") ? std::optional{&customData["_position"]} : std::nullopt;
    std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
    std::optional<float> njs = customData.HasMember("_noteJumpMovementSpeed") ? std::optional{customData["_noteJumpMovementSpeed"].GetFloat()} : std::nullopt;
    std::optional<float> spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? std::optional{customData["_noteJumpStartBeatOffset"].GetFloat()} : std::nullopt;

    std::optional<float> startX = position.has_value() ? std::optional{(*position.value())[0].GetFloat()} : std::nullopt;
    std::optional<float> startY = position.has_value() ? std::optional{(*position.value())[1].GetFloat()} : std::nullopt;

    std::optional<float> height = scale.has_value() ? std::optional{(*scale.value())[1].GetFloat()} : std::nullopt;

    UnityEngine::Vector3 moveStartPos = result.moveStartPos;
    UnityEngine::Vector3 moveEndPos = result.moveEndPos;
    UnityEngine::Vector3 jumpEndPos = result.jumpEndPos;
    float obstacleHeight = result.obstacleHeight;

    float jumpDuration;
    float jumpDistance;
    UnityEngine::Vector3 localMoveStartPos;
    UnityEngine::Vector3 localMoveEndPos;
    UnityEngine::Vector3 localJumpEndPos;
    SpawnDataHelper::GetNoteJumpValues(self, njs, spawnOffset, jumpDuration, jumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);

    std::optional<UnityEngine::Vector3> finalNoteOffset = std::nullopt;

    if (startX.has_value() || startY.has_value() || njs.has_value() || spawnOffset.has_value()) {
        UnityEngine::Vector3 noteOffset = SpawnDataHelper::GetNoteOffset(self, obstacleData, startX, std::nullopt);
        noteOffset.y = startY.has_value() ? self->verticalObstaclePosY + (startY.value() * self->noteLinesDistance) : ((obstacleData->obstacleType == ObstacleType::Top)
            ? (self->topObstaclePosY + self->jumpOffsetY) : self->verticalObstaclePosY);

        finalNoteOffset = std::optional{noteOffset};

        moveStartPos = localMoveStartPos + noteOffset;
        moveEndPos = localMoveEndPos + noteOffset;
        jumpEndPos = localJumpEndPos + noteOffset;
    }

    if (height.has_value()) {
        obstacleHeight = height.value() * self->noteLinesDistance;
    }

    result = BeatmapObjectSpawnMovementData_ObstacleSpawnData(moveStartPos, moveEndPos, jumpEndPos, obstacleHeight, result.moveDuration, jumpDuration, self->noteLinesDistance);
    // result = BeatmapObjectSpawnMovementData_ObstacleSpawnData(UnityEngine::Vector3 {1, 2, 3}, UnityEngine::Vector3 {4, 5, 6}, UnityEngine::Vector3 {7, 8, 9}, obstacleHeight, result.moveDuration, jumpDuration, self->noteLinesDistance);
    
    return result;
}

struct BeatmapObjectSpawnMovementData_NoteSpawnData : public System::ValueType {
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    float jumpGravity;
    float moveDuration;
    float jumpDuration;
    constexpr BeatmapObjectSpawnMovementData_NoteSpawnData(UnityEngine::Vector3 moveStartPos_ = {}, UnityEngine::Vector3 moveEndPos_ = {}, UnityEngine::Vector3 jumpEndPos_ = {}, float jumpGravity_ = {}, float moveDuration_ = {}, float jumpDuration_ = {}) noexcept : moveStartPos{moveStartPos_}, moveEndPos{moveEndPos_}, jumpEndPos{jumpEndPos_}, jumpGravity{jumpGravity_}, moveDuration{moveDuration_}, jumpDuration{jumpDuration_} {}
};

MAKE_HOOK_OFFSETLESS(GetJumpingNoteSpawnData, BeatmapObjectSpawnMovementData_NoteSpawnData, BeatmapObjectSpawnMovementData *self, CustomJSONData::CustomNoteData *noteData) {
    BeatmapObjectSpawnMovementData_NoteSpawnData result = GetJumpingNoteSpawnData(self, noteData);
    if (!noteData->customData) {
        return result;
    }

    rapidjson::Value &customData = *noteData->customData;

    std::optional<rapidjson::Value*> position = customData.HasMember("_position") ? std::optional{&customData["_position"]} : std::nullopt;
    std::optional<float> flipLineIndex = customData.HasMember("flipLineIndex") ? std::optional{customData["flipLineIndex"].GetFloat()} : std::nullopt;
    std::optional<float> njs = customData.HasMember("_noteJumpMovementSpeed") ? std::optional{customData["_noteJumpMovementSpeed"].GetFloat()} : std::nullopt;
    std::optional<float> spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? std::optional{customData["_noteJumpStartBeatOffset"].GetFloat()} : std::nullopt;
    std::optional<float> startLineLayer = customData.HasMember("startNoteLineLayer") ? std::optional{customData["startNoteLineLayer"].GetFloat()} : std::nullopt;

    bool gravityOverride = customData.HasMember("_disableNoteGravity") ? customData["_disableNoteGravity"].GetBool() : false;

    std::optional<float> startRow = position.has_value() ? std::optional{(*position.value())[0].GetFloat()} : std::nullopt;
    std::optional<float> startHeight = position.has_value() ? std::optional{(*position.value())[1].GetFloat()} : std::nullopt;

    float jumpDuration = self->jumpDuration;

    UnityEngine::Vector3 moveStartPos = result.moveStartPos;
    UnityEngine::Vector3 moveEndPos = result.moveEndPos;
    UnityEngine::Vector3 jumpEndPos = result.jumpEndPos;
    float jumpGravity = result.jumpGravity;

    UnityEngine::Vector3 noteOffset = SpawnDataHelper::GetNoteOffset(self, noteData, startRow, startLineLayer.value_or(noteData->startNoteLineLayer));

    if (position.has_value() || flipLineIndex.has_value() || njs.has_value() || spawnOffset.has_value() || startLineLayer.has_value() || gravityOverride) {
        float localJumpDuration;
        float localJumpDistance;
        UnityEngine::Vector3 localMoveStartPos;
        UnityEngine::Vector3 localMoveEndPos;
        UnityEngine::Vector3 localJumpEndPos;
        SpawnDataHelper::GetNoteJumpValues(self, njs, spawnOffset, localJumpDuration, localJumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);
        jumpDuration = localJumpDuration;

        float localNoteJumpMovementSpeed = njs.value_or(self->noteJumpMovementSpeed);

        float startLayerLineYPos = SpawnDataHelper::LineYPosForLineLayer(self, noteData, startLineLayer.value_or(noteData->startNoteLineLayer));
        float lineYPos = SpawnDataHelper::LineYPosForLineLayer(self, noteData, startHeight);

        float highestJump = startHeight.has_value() ? (0.875f * lineYPos) + 0.639583f + self->jumpOffsetY :
            self->HighestJumpPosYForLineLayer(noteData->noteLineLayer);
        jumpGravity = 2.0f * (highestJump - (gravityOverride ? lineYPos : startLayerLineYPos)) /
            std::pow(localJumpDistance / localNoteJumpMovementSpeed * 0.5f, 2.0f);

        jumpEndPos = localJumpEndPos + noteOffset;

        std::optional<float> offsetStartRow = flipLineIndex.has_value() ? flipLineIndex : startRow;
        std::optional<float> offsetStartHeight = gravityOverride ? startHeight : (startLineLayer.has_value() ? startLineLayer : std::optional{(float) noteData->startNoteLineLayer.value});
        UnityEngine::Vector3 noteOffset2 = SpawnDataHelper::GetNoteOffset(self, noteData, offsetStartRow, offsetStartHeight);
        moveStartPos = localMoveStartPos + noteOffset2;
        moveEndPos = localMoveEndPos + noteOffset2;

        result = BeatmapObjectSpawnMovementData_NoteSpawnData(moveStartPos, moveEndPos, jumpEndPos, jumpGravity, result.moveDuration, jumpDuration);
    }

    return result;
}

void NoodleExtensions::InstallBeatmapObjectSpawnMovementDataHooks() {
    INSTALL_HOOK_OFFSETLESS(GetObstacleSpawnData, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnMovementData", "GetObstacleSpawnData", 1));
    // INSTALL_HOOK_OFFSETLESS(GetJumpingNoteSpawnData, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnMovementData", "GetJumpingNoteSpawnData", 1));
}