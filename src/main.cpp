#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "custom-types/shared/macros.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "System/Collections/Generic/List_1.hpp"

#include "CustomJSONData/CustomBeatmapSaveData.h"
#include "CustomJSONData/CustomBeatmapData.h"
#include "NELogger.h"

#include <string>
#include <iostream>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

// This is to prevent issues with string limits
std::string to_utf8(std::u16string_view view) {
    char* dat = static_cast<char*>(calloc(view.length() + 1, sizeof(char)));
    std::transform(view.data(), view.data() + view.size(), dat, [](auto utf16_char) {
        return static_cast<char>(utf16_char);
    });
    dat[view.length()] = '\0';
    return {dat};
}

// This hook loads the json data (with custom data) into a BeatmapSaveData 
MAKE_HOOK_OFFSETLESS(DeserializeFromJSONString, BeatmapSaveData*, Il2CppString *stringData) {
    NELogger::GetLogger().debug("Parsing json");

    std::string str = to_utf8(csstrtostr(stringData));
    
    rapidjson::Document doc;
    doc.Parse(str.c_str());

    NELogger::GetLogger().debug("Parsing json success");
    
    List_1<BeatmapSaveData::NoteData*> *notes = List_1<BeatmapSaveData::NoteData*>::New_ctor();
    List_1<BeatmapSaveData::ObstacleData*> *obstacles = List_1<BeatmapSaveData::ObstacleData*>::New_ctor();
    List_1<BeatmapSaveData::EventData*> *events = List_1<BeatmapSaveData::EventData*>::New_ctor();
    
    NELogger::GetLogger().debug("Parse notes");
    // Parse notes
    rapidjson::Value& notes_arr = doc["_notes"];
    for (rapidjson::SizeType i = 0; i < notes_arr.Size(); i++) {
        rapidjson::Value& note_json = notes_arr[i];

        float time = note_json["_time"].GetFloat();
        int lineIndex = note_json["_lineIndex"].GetInt();
        NoteLineLayer lineLayer = NoteLineLayer(note_json["_lineLayer"].GetInt());
        BeatmapSaveData::NoteType type = BeatmapSaveData::NoteType(note_json["_type"].GetInt());
        NoteCutDirection cutDirection = NoteCutDirection(note_json["_cutDirection"].GetInt());
        auto note = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapSaveData_NoteData*>(time, lineIndex, lineLayer, type, cutDirection));
        if (note_json.HasMember("_customData")) {
            note->customData = new rapidjson::Value(note_json["_customData"], doc.GetAllocator());
        }
        notes->Add(note);
    }

    NELogger::GetLogger().debug("Parse obstacles");
    // Parse obstacles
    rapidjson::Value& obstacles_arr = doc["_obstacles"];
    for (rapidjson::SizeType i = 0; i < obstacles_arr.Size(); i++) {
        rapidjson::Value& obstacle_json = obstacles_arr[i];

        float time = obstacle_json["_time"].GetFloat();
        int lineIndex = obstacle_json["_lineIndex"].GetInt();
        ObstacleType type = ObstacleType(obstacle_json["_type"].GetInt());
        float duration = obstacle_json["_duration"].GetFloat();
        int width = obstacle_json["_width"].GetInt();
        auto obstacle = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapSaveData_ObstacleData*>(time, lineIndex, type, duration, width));
        if (obstacle_json.HasMember("_customData")) {
            obstacle->customData = new rapidjson::Value(obstacle_json["_customData"], doc.GetAllocator());
        }
        obstacles->Add(obstacle);
    }

    NELogger::GetLogger().debug("Parse events");
    // Parse events
    rapidjson::Value& events_arr = doc["_events"];
    for (rapidjson::SizeType i = 0; i < events_arr.Size(); i++) {
        rapidjson::Value& event_json = events_arr[i];
        
        float time = event_json["_time"].GetFloat();
        BeatmapSaveData::BeatmapEventType type = BeatmapSaveData::BeatmapEventType(event_json["_type"].GetInt());
        int value = event_json["_value"].GetInt();
        auto event = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapSaveData_EventData*>(time, type, value));
        if (event_json.HasMember("_customData")) {
            event->customData = new rapidjson::Value(event_json["_customData"], doc.GetAllocator());
        } 
        events->Add(event);
    }

    NELogger::GetLogger().debug("Parse root custom");
    auto saveData = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapSaveData*>(events, notes, obstacles));
    if (doc.HasMember("_customData")) {
        saveData->customData = new rapidjson::Value(doc["_customData"], doc.GetAllocator());
    }

    NELogger::GetLogger().debug("Finished reading beatmap data");

    return saveData;
}

CustomJSONData::CustomNoteData* CustomJSONDataCreateBasicNoteData(float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection) {
    NELogger::GetLogger().info("Create Basic Note Data");
    NELogger::GetLogger().debug("custom klass: %p", CustomJSONData::CustomNoteData::klass);
    return CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomNoteData*>(time, lineIndex, noteLineLayer, noteLineLayer, colorType, cutDirection, 0.0f, 0.0f, lineIndex, 0.0f, 0.0f));
}

CustomJSONData::CustomNoteData* CustomJSONDataCreateBombNoteData(float time, int lineIndex, NoteLineLayer noteLineLayer) {
    
    return CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomNoteData*>(time, lineIndex, noteLineLayer, noteLineLayer, ColorType::None, NoteCutDirection::None, 0.0f, 0.0f, lineIndex, 0.0f, 0.0f));
}

// This hook creates the CustomNoteData using the custom json data found in the BeatmapSaveData
MAKE_HOOK_OFFSETLESS(CreateBasicNoteData, NoteData*, float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection) {
    CustomJSONData::CustomBeatmapSaveData_NoteData *noteData;

    // Since I don't feel like patching the original assembly to call my own function
    // with an extra noteData parameter, I get it by reading the register where it should be.
    // WARNING: This will most likely break on every update as noteData might be on another register. 
    //          If a hook is ran before this one and it overwrites this register, then this will most 
    //          likely break.
    asm ("mov %[result], x23"
        : [result] "=r" (noteData));

    auto result = CustomJSONDataCreateBasicNoteData(time, lineIndex, noteLineLayer, colorType, cutDirection);
    result->customData = noteData->customData;
    return result;
}

// This hook creates the CustomNoteData for bombs using the custom json data found in the BeatmapSaveData
MAKE_HOOK_OFFSETLESS(CreateBombNoteData, NoteData*, float time, int lineIndex, NoteLineLayer noteLineLayer) {
    CustomJSONData::CustomBeatmapSaveData_NoteData *noteData;

    // This register number should be the same as the one used in CreateBasicNoteData
    asm ("mov %[result], x23"
        : [result] "=r" (noteData));

    auto result = CustomJSONDataCreateBombNoteData(time, lineIndex, noteLineLayer);
    result->customData = noteData->customData;
    return result;
}

UnityEngine::Vector3 GetNoteOffset(BeatmapObjectSpawnMovementData *spawnMovementData, BeatmapObjectData *beatmapObjectData, std::optional<float> startRow, std::optional<float> startHeight) {
    float distance = (-(spawnMovementData->noteLinesCount - 1) * 0.5) + (startRow.has_value() ? spawnMovementData->noteLinesCount / 2 : 0);
    float lineIndex = startRow.value_or(0);
    distance = (distance + lineIndex) * spawnMovementData->noteLinesDistance;

    return (spawnMovementData->rightVec * distance) + UnityEngine::Vector3(0, 0, 0);
}

void GetNoteJumpValues(BeatmapObjectSpawnMovementData *spawnMovementData, std::optional<float> inputNoteJumpMovementSpeed, std::optional<float> inputNoteJumpStartBeatOffset, float &localJumpDuration, 
                       float &localJumpDistance, UnityEngine::Vector3 &localMoveStartPos, UnityEngine::Vector3 localMoveEndPos, UnityEngine::Vector3 localJumpEndPos) {
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
    if (obstacleData->customData == nullptr) {
        return result;
    }
    rapidjson::Value &customData = *obstacleData->customData;

    std::optional<rapidjson::Value*> position = customData.HasMember("_position") ? std::optional<rapidjson::Value*>{&customData["_position"]} : std::nullopt;
    std::optional<float> njs = customData.HasMember("_noteJumpMovementSpeed") ? std::optional<float>{customData["_noteJumpMovementSpeed"].IsFloat()} : std::nullopt;
    std::optional<float> spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? std::optional<float>{customData["_noteJumpStartBeatOffset"].IsFloat()} : std::nullopt;

    std::optional<float> startX = position.has_value() ? std::optional<float>{(*position.value())[0].IsFloat()} : std::nullopt;
    std::optional<float> startY = position.has_value() ? std::optional<float>{(*position.value())[1].IsFloat()} : std::nullopt;

    UnityEngine::Vector3 moveStartPos = result.moveStartPos;
    UnityEngine::Vector3 moveEndPos = result.moveEndPos;
    UnityEngine::Vector3 jumpEndPos = result.jumpEndPos;

    float localJumpDuration;
    float localJumpDistance;
    UnityEngine::Vector3 localMoveStartPos;
    UnityEngine::Vector3 localMoveEndPos;
    UnityEngine::Vector3 localJumpEndPos;
    GetNoteJumpValues(self, njs, spawnOffset, localJumpDuration, localJumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);

    UnityEngine::Vector3 finalNoteOffset;

    if (startX.has_value() || startY.has_value() || njs.has_value() || spawnOffset.has_value()) {
        UnityEngine::Vector3 noteOffset = GetNoteOffset(self, obstacleData, startX, std::nullopt);

        finalNoteOffset = noteOffset;

        moveStartPos = localMoveStartPos + noteOffset;
        moveEndPos = localMoveEndPos + noteOffset;
        jumpEndPos = localJumpEndPos + noteOffset;
    }

    return result;
}

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = "0.1.0";
    NELogger::modInfo = info;
}

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");

    Logger::get().options.silent = true;

    // Install hooks
    INSTALL_HOOK_OFFSETLESS(DeserializeFromJSONString, il2cpp_utils::FindMethodUnsafe("", "BeatmapSaveData", "DeserializeFromJSONString", 1));
    INSTALL_HOOK_OFFSETLESS(CreateBasicNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBasicNoteData", 5));
    INSTALL_HOOK_OFFSETLESS(CreateBombNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBombNoteData", 3));

    // Register custom tpes
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapSaveData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapSaveData_NoteData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapSaveData_ObstacleData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapSaveData_EventData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomBeatmapEventData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomObstacleData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomNoteData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<CustomJSONData::CustomEventData>());

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}