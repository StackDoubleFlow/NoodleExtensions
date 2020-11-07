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
#include "System/Collections/Generic/List_1.hpp"

#include "CustomBeatmapSaveData.h"
#include "CustomBeatmapData.h"

#include <string>
#include <iostream>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

static ModInfo modInfo;

const Logger &getLogger() {
    static const Logger &logger(modInfo);
    return logger;
}

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
    getLogger().debug("Parsing json");

    std::string str = to_utf8(csstrtostr(stringData));
    
    rapidjson::Document doc;
    doc.Parse(str.c_str());

    getLogger().debug("Parsing json success");
    
    List_1<BeatmapSaveData::NoteData*> *notes = List_1<BeatmapSaveData::NoteData*>::New_ctor();
    List_1<BeatmapSaveData::ObstacleData*> *obstacles = List_1<BeatmapSaveData::ObstacleData*>::New_ctor();
    List_1<BeatmapSaveData::EventData*> *events = List_1<BeatmapSaveData::EventData*>::New_ctor();
    
    getLogger().debug("Parse notes");
    // Parse notes
    rapidjson::Value& notes_arr = doc["_notes"];
    for (rapidjson::SizeType i = 0; i < notes_arr.Size(); i++) {
        rapidjson::Value& note_json = notes_arr[i];

        float time = note_json["_time"].GetFloat();
        int lineIndex = note_json["_lineIndex"].GetInt();
        NoteLineLayer lineLayer = NoteLineLayer(note_json["_lineLayer"].GetInt());
        BeatmapSaveData::NoteType type = BeatmapSaveData::NoteType(note_json["_type"].GetInt());
        NoteCutDirection cutDirection = NoteCutDirection(note_json["_cutDirection"].GetInt());
        auto note = CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomBeatmapSaveData_NoteData*>(time, lineIndex, lineLayer, type, cutDirection));
        if (note_json.HasMember("_customData")) {
            note->customData = new rapidjson::Value(note_json["_customData"], doc.GetAllocator());
        }
        notes->Add(note);
    }

    getLogger().debug("Parse obstacles");
    // Parse obstacles
    rapidjson::Value& obstacles_arr = doc["_obstacles"];
    for (rapidjson::SizeType i = 0; i < obstacles_arr.Size(); i++) {
        rapidjson::Value& obstacle_json = obstacles_arr[i];

        float time = obstacle_json["_time"].GetFloat();
        int lineIndex = obstacle_json["_lineIndex"].GetInt();
        ObstacleType type = ObstacleType(obstacle_json["_type"].GetInt());
        float duration = obstacle_json["_duration"].GetFloat();
        int width = obstacle_json["_width"].GetInt();
        auto obstacle = CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomBeatmapSaveData_ObstacleData*>(time, lineIndex, type, duration, width));
        if (obstacle_json.HasMember("_customData")) {
            obstacle->customData = new rapidjson::Value(obstacle_json["_customData"], doc.GetAllocator());
        }
        obstacles->Add(obstacle);
    }

    getLogger().debug("Parse events");
    // Parse events
    rapidjson::Value& events_arr = doc["_events"];
    for (rapidjson::SizeType i = 0; i < events_arr.Size(); i++) {
        rapidjson::Value& event_json = events_arr[i];
        
        float time = event_json["_time"].GetFloat();
        BeatmapSaveData::BeatmapEventType type = BeatmapSaveData::BeatmapEventType(event_json["_type"].GetInt());
        int value = event_json["_value"].GetInt();
        auto event = CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomBeatmapSaveData_EventData*>(time, type, value));
        if (event_json.HasMember("_customData")) {
            event->customData = new rapidjson::Value(event_json["_customData"], doc.GetAllocator());
        } 
        events->Add(event);
    }

    getLogger().debug("Parse root custom");
    auto saveData = CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomBeatmapSaveData*>(events, notes, obstacles));
    if (doc.HasMember("_customData")) {
        saveData->customData = new rapidjson::Value(doc["_customData"], doc.GetAllocator());
    }

    getLogger().debug("Finished reading beatmap data");

    return saveData;
}


// This hook creates the CustomNoteData using the custom json data found in the BeatmapSaveData
MAKE_HOOK_OFFSETLESS(CreateBasicNoteData, NoteData*, float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection) {
    Il2CppNamespace::CustomBeatmapSaveData_NoteData *noteData;

    // Since I don't feel like patching the original assembly to call my own function
    // with an extra noteData parameter, I get it by reading the register where it should be.
    // WARNING: This will most likely break on every update as noteData might be on another register. 
    //          If a hook is ran before this one and it overwrites this register, then this will most 
    //          likely break.
    asm ("mov %[result], x23"
        : [result] "=r" (noteData));

    auto result = Il2CppNamespace::CustomNoteData::CreateBasicNoteData(time, lineIndex, noteLineLayer, colorType, cutDirection);
    result->customData = noteData->customData;
    return result;
}

// This hook creates the CustomNoteData for bombs using the custom json data found in the BeatmapSaveData
MAKE_HOOK_OFFSETLESS(CreateBombNoteData, NoteData*, float time, int lineIndex, NoteLineLayer noteLineLayer) {
    Il2CppNamespace::CustomBeatmapSaveData_NoteData *noteData;

    // This register number should be the same as the one used in CreateBasicNoteData
    asm ("mov %[result], x23"
        : [result] "=r" (noteData));

    auto result = Il2CppNamespace::CustomNoteData::CreateBombNoteData(time, lineIndex, noteLineLayer);
    result->customData = noteData->customData;
    return result;
}

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = "0.1.0";
    modInfo = info;
}

extern "C" void load() {
    getLogger().info("Installing NoodleExtensions Hooks!");

    Logger::get().options.silent = true;

    // Install hooks
    INSTALL_HOOK_OFFSETLESS(DeserializeFromJSONString, il2cpp_utils::FindMethodUnsafe("", "BeatmapSaveData", "DeserializeFromJSONString", 1));
    INSTALL_HOOK_OFFSETLESS(CreateBasicNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBasicNoteData", 5));
    INSTALL_HOOK_OFFSETLESS(CreateBombNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBombNoteData", 3));

    // Register custom tpes
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapSaveData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapSaveData_NoteData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapSaveData_ObstacleData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapSaveData_EventData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomBeatmapEventData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomObstacleData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomNoteData>());
    CRASH_UNLESS(custom_types::Register::RegisterType<Il2CppNamespace::CustomEventData>());
    

    for (int i = 0; i < custom_types::Register::classes.size(); i++) {
        getLogger().info("%p", custom_types::Register::classes[i]);
    }

    getLogger().info("Installed NoodleExtensions Hooks!");
}