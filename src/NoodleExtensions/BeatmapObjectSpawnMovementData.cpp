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
#include "NoodleExtensions/SpawnDataHelper.h"
#include "NoodleExtensions/NEHooks.h"
#include "NELogger.h"

using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(GetObstacleSpawnData, BeatmapObjectSpawnMovementData::ObstacleSpawnData, BeatmapObjectSpawnMovementData *self, CustomJSONData::CustomObstacleData *obstacleData) {
    BeatmapObjectSpawnMovementData::ObstacleSpawnData result = GetObstacleSpawnData(self, obstacleData);

    NELogger::GetLogger().info("GetObstacleSpawnData obstacleData->customData pointer: %p", obstacleData->getCustomData());
    // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
    if (!obstacleData->getCustomData()) {
        return result;
    }

    // Debug json by pretty printing it
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    obstacleData->getCustomData()->Accept(writer);
    const char* json = buffer.GetString();
    NELogger::GetLogger().info("custom data: %s", json);

    rapidjson::Value &customData = *obstacleData->getCustomData();

    std::optional<rapidjson::Value*> position = customData.HasMember("_position") ? std::optional{&customData["_position"]} : std::nullopt;
    std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
    std::optional<float> njs = customData.HasMember("_noteJumpMovementSpeed") ? std::optional{customData["_noteJumpMovementSpeed"].IsFloat()} : std::nullopt;
    std::optional<float> spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? std::optional{customData["_noteJumpStartBeatOffset"].IsFloat()} : std::nullopt;

    std::optional<float> startX = position.has_value() ? std::optional{(*position.value())[0].IsFloat()} : std::nullopt;
    std::optional<float> startY = position.has_value() ? std::optional{(*position.value())[1].IsFloat()} : std::nullopt;

    std::optional<float> height = scale.has_value() ? std::optional{(*scale.value())[1].IsFloat()} : std::nullopt;

    UnityEngine::Vector3 moveStartPos = result.moveStartPos;
    UnityEngine::Vector3 moveEndPos = result.moveEndPos;
    UnityEngine::Vector3 jumpEndPos = result.jumpEndPos;

    float jumpDuration;
    float jumpDistance;
    UnityEngine::Vector3 localMoveStartPos;
    UnityEngine::Vector3 localMoveEndPos;
    UnityEngine::Vector3 localJumpEndPos;
    float obstacleHeight = result.obstacleHeight;
    SpawnDataHelper::GetNoteJumpValues(self, njs, spawnOffset, jumpDuration, jumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);

    std::optional<UnityEngine::Vector3> finalNoteOffset = std::nullopt;

    if (startX.has_value() || startY.has_value() || njs.has_value() || spawnOffset.has_value()) {
        UnityEngine::Vector3 noteOffset = SpawnDataHelper::GetNoteOffset(self, obstacleData, startX, std::nullopt);

        finalNoteOffset = std::optional{noteOffset};

        moveStartPos = localMoveStartPos + noteOffset;
        moveEndPos = localMoveEndPos + noteOffset;
        jumpEndPos = localJumpEndPos + noteOffset;
    }

    if (height.has_value()) {
        obstacleHeight = height.value() * self->noteLinesDistance;
    }

    result = BeatmapObjectSpawnMovementData::ObstacleSpawnData(moveStartPos, moveEndPos, obstacleHeight, result.moveDuration, jumpDuration, result.noteLinesDistance);


    return result;
}

void NoodleExtensions::InstallBeatmapObjectSpawnMovementDataHooks() {
    INSTALL_HOOK_OFFSETLESS(GetObstacleSpawnData, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnMovementData", "GetObstacleSpawnData", 1));
}