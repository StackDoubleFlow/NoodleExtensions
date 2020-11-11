#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/BeatmapObjectType.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "System/Collections/Generic/List_1.hpp"

#include "CustomJSONData/CustomBeatmapSaveData.h"
#include "CustomJSONData/CustomBeatmapData.h"
#include "CustomJSONData/CustomJSONDataHooks.h"
#include "NELogger.h"

#include <string>
#include <iostream>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

UnityEngine::Vector3 GetNoteOffset(BeatmapObjectSpawnMovementData *spawnMovementData, BeatmapObjectData *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight) {
    float distance = (-(spawnMovementData->noteLinesCount - 1) * 0.5) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2 : 0);
    float lineIndex = startRow.value_or(0);
    distance = (distance + lineIndex) * spawnMovementData->noteLinesDistance;

    return (spawnMovementData->rightVec * distance) + UnityEngine::Vector3(0, 0, 0);
}

void GetNoteJumpValues(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration, 
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
    localMoveEndPos = spawnMovementData->centerPos + (spawnMovementData->forwardVec * localJumpDistance * 0.5);
    localJumpEndPos = spawnMovementData->centerPos - (spawnMovementData->forwardVec * localJumpDistance * 0.5);
}

MAKE_HOOK_OFFSETLESS(GetObstacleSpawnData, BeatmapObjectSpawnMovementData::ObstacleSpawnData, BeatmapObjectSpawnMovementData *self, CustomJSONData::CustomObstacleData *obstacleData) {
    BeatmapObjectSpawnMovementData::ObstacleSpawnData result = GetObstacleSpawnData(self, obstacleData);

    NELogger::GetLogger().info("GetObstacleSpawnData obstacleData->customData pointer: %p", obstacleData->customData);
    // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
    if (!obstacleData->customData) {
        return result;
    }

    // Debug json by pretty printing it
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    obstacleData->customData->Accept(writer);
    const char* json = buffer.GetString();
    NELogger::GetLogger().info("custom data: %s", json);

    rapidjson::Value &customData = *obstacleData->customData;

    std::optional<rapidjson::Value*> position = customData.HasMember("_position") ? std::optional<rapidjson::Value*>{&customData["_position"]} : std::nullopt;
    std::optional<float> njs = customData.HasMember("_noteJumpMovementSpeed") ? std::optional<float>{customData["_noteJumpMovementSpeed"].IsFloat()} : std::nullopt;
    std::optional<float> spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? std::optional<float>{customData["_noteJumpStartBeatOffset"].IsFloat()} : std::nullopt;

    std::optional<float> startX = position.has_value() ? std::optional<float>{(*position.value())[0].IsFloat()} : std::nullopt;
    std::optional<float> startY = position.has_value() ? std::optional<float>{(*position.value())[1].IsFloat()} : std::nullopt;

    UnityEngine::Vector3 moveStartPos = result.moveStartPos;
    UnityEngine::Vector3 moveEndPos = result.moveEndPos;
    UnityEngine::Vector3 jumpEndPos = result.jumpEndPos;

    float jumpDuration;
    float jumpDistance;
    UnityEngine::Vector3 localMoveStartPos;
    UnityEngine::Vector3 localMoveEndPos;
    UnityEngine::Vector3 localJumpEndPos;
    float obstacleHeight = result.obstacleHeight;
    GetNoteJumpValues(self, njs, spawnOffset, jumpDuration, jumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);

    UnityEngine::Vector3 finalNoteOffset;

    if (startX.has_value() || startY.has_value() || njs.has_value() || spawnOffset.has_value()) {
        UnityEngine::Vector3 noteOffset = GetNoteOffset(self, obstacleData, startX, std::nullopt);

        finalNoteOffset = noteOffset;

        moveStartPos = localMoveStartPos + noteOffset;
        moveEndPos = localMoveEndPos + noteOffset;
        jumpEndPos = localJumpEndPos + noteOffset;
    }

    result = BeatmapObjectSpawnMovementData::ObstacleSpawnData(moveStartPos, moveEndPos, obstacleHeight, result.moveDuration, jumpDuration, result.noteLinesDistance);

    return result;
}

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = "0.1.0";
    NELogger::modInfo = info;
}

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");

    // This prevents any and all Utils logging
    Logger::get().options.silent = true;

    // Install hooks
    INSTALL_HOOK_OFFSETLESS(GetObstacleSpawnData, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnMovementData", "GetObstacleSpawnData", 1));

    CustomJSONData::InstallHooks();

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}