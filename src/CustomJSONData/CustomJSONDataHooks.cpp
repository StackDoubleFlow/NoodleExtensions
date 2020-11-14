#include "custom-types/shared/register.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"

#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_WaypointData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventKeywordFiltersData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventsForKeyword.hpp"
#include "GlobalNamespace/BeatmapDataLoader_BpmChangeData.hpp"
#include "GlobalNamespace/BeatmapObjectType.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "System/Comparison_1.hpp"

#include "CustomJSONData/CustomBeatmapSaveData.h"
#include "CustomJSONData/CustomBeatmapData.h"
#include "CustomJSONData/CustomJSONDataHooks.h"
#include "NELogger.h"

using namespace GlobalNamespace;

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

    auto alloc = new rapidjson::MemoryPoolAllocator();

    NELogger::GetLogger().debug("Parsing json success");
    
    List_1<BeatmapSaveData::NoteData*> *notes = List_1<BeatmapSaveData::NoteData*>::New_ctor();
    List_1<BeatmapSaveData::ObstacleData*> *obstacles = List_1<BeatmapSaveData::ObstacleData*>::New_ctor();
    List_1<BeatmapSaveData::EventData*> *events = List_1<BeatmapSaveData::EventData*>::New_ctor();
    List_1<BeatmapSaveData::WaypointData*> *waypoints = List_1<BeatmapSaveData::WaypointData*>::New_ctor();
    
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
            note->customData = new rapidjson::Value(note_json["_customData"], *alloc);
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
        NELogger::GetLogger().debug("obstacle pointer: %p", obstacle);
        if (obstacle_json.HasMember("_customData")) {
            obstacle->customData = new rapidjson::Value(obstacle_json["_customData"], *alloc);
            NELogger::GetLogger().debug("obstacle->customData pointer: %p", obstacle->customData);
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
            event->customData = new rapidjson::Value(event_json["_customData"], *alloc);
        } 
        events->Add(event);
    }

    NELogger::GetLogger().debug("Parse waypoints");
    rapidjson::Value& waypoints_arr = doc["_waypoints"];
    for (rapidjson::SizeType i = 0; i < waypoints_arr.Size(); i++) {
        rapidjson::Value& waypoint_json = waypoints_arr[i];
        
        float time = waypoint_json["_time"].GetFloat();
        int lineIndex = waypoint_json["_lineIndex"].GetInt();
        NoteLineLayer lineLayer = NoteLineLayer(waypoint_json["_lineLayer"].GetInt());
        OffsetDirection offsetDirection = OffsetDirection(waypoint_json["_offsetDirection"].GetInt());
        auto waypoint = BeatmapSaveData::WaypointData::New_ctor(time, lineIndex, lineLayer, offsetDirection);
        waypoints->Add(waypoint);
    }

    // TODO: Parse whatever the hell this is
    auto specialEventsKeywordFilters = BeatmapSaveData::SpecialEventKeywordFiltersData::New_ctor(List_1<BeatmapSaveData::SpecialEventsForKeyword*>::New_ctor());

    NELogger::GetLogger().debug("Parse root");
    auto saveData = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapSaveData*>(events, notes, waypoints, obstacles, specialEventsKeywordFilters));
    if (doc.HasMember("_customData")) {
        saveData->customData = new rapidjson::Value(doc["_customData"], *alloc);
    }

    NELogger::GetLogger().debug("Finished reading beatmap data");

    return saveData;
}

CustomJSONData::CustomNoteData* CustomJSONDataCreateBasicNoteData(float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection) {
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
    // result->customData = noteData->customData;
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

// This hook is primarily used to create CustomObstacleData by checking if it is adding a regular ObstacleData and converting it
MAKE_HOOK_OFFSETLESS(AddBeatmapObjectData, void, BeatmapData *self, BeatmapObjectData *beatmapObjectData) {
    CustomJSONData::CustomBeatmapSaveData_ObstacleData *saveObstacleData;
    // This asm will only work correctly if the methods was called from GetBeatmapDataFromBeatmapSaveData
    asm ("mov %[result], x25"
        : [result] "=r" (saveObstacleData));
    // This is to prevent crashes with the hook being called from the unintended functions
    if (std::strncmp(beatmapObjectData->klass->name, "ObstacleData", 13)) {
        return AddBeatmapObjectData(self, beatmapObjectData);
    }
    NELogger::GetLogger().info("saveObstacleData->klass->name: %s", saveObstacleData->klass->name);
    NELogger::GetLogger().info("AddBeatmapObjectData saveObstacleData pointer: %p", saveObstacleData);
    NELogger::GetLogger().info("AddBeatmapObjectData customData pointer: %p", saveObstacleData->customData);
    NELogger::GetLogger().info("AddBeatmapObjectData type: %i", beatmapObjectData->get_beatmapObjectType());
    if (saveObstacleData->customData) {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        saveObstacleData->customData->Accept(writer);
        const char* json = buffer.GetString();
        NELogger::GetLogger().info("custom data: %s", json);
    }
    if (beatmapObjectData->get_beatmapObjectType() == BeatmapObjectType::Obstacle) {
        ObstacleData *obstacleData = (ObstacleData *) beatmapObjectData;
        CustomJSONData::CustomObstacleData *customObstacleData = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomObstacleData*>(obstacleData->time, obstacleData->lineIndex, obstacleData->obstacleType, obstacleData->duration, obstacleData->width));
        // customObstacleData->customData = saveObstacleData->customData;
        return AddBeatmapObjectData(self, customObstacleData);
    }
    return AddBeatmapObjectData(self, beatmapObjectData);
}

float ProcessTime(BeatmapDataLoader *self, float bpmTime, int &bpmChangesDataIdx,List_1<BeatmapDataLoader::BpmChangeData> *bpmChangesData, float shuffle, float shufflePeriod) {
    while(bpmChangesDataIdx < bpmChangesData->get_Count() - 1 && bpmChangesData->get_Item(bpmChangesDataIdx + 1).bpmChangeStartBpmTime < bpmTime) {
        bpmChangesDataIdx++;
    }
    BeatmapDataLoader::BpmChangeData bpmChangeData = bpmChangesData->get_Item(bpmChangesDataIdx);
    return bpmChangeData.bpmChangeStartTime + self->GetRealTimeFromBPMTime(bpmTime - bpmChangeData.bpmChangeStartBpmTime, bpmChangeData.bpm, shuffle, shufflePeriod);
}

MAKE_HOOK_OFFSETLESS(GetBeatmapDataFromBeatmapSaveData, BeatmapData *, BeatmapDataLoader *self, List_1<BeatmapSaveData::NoteData *> *notesSaveData, List_1<BeatmapSaveData::WaypointData *> *waypointsSaveData, 
    List_1<BeatmapSaveData::ObstacleData *> *obstaclesSaveData, List_1<BeatmapSaveData::EventData *> *eventsSaveData, BeatmapSaveData::SpecialEventKeywordFiltersData *evironmentSpecialEventFilterData, float startBpm, float shuffle, float shufflePeriod) {
    // int length = obstaclesSaveData->get_Count();
    // for (int i = 0; i < length; i++) {
    //     auto obstacle = obstaclesSaveData->get_Item(i);
    //     NELogger::GetLogger().info("GetBeatmapDataFromBeatmapSaveData obstacle pointer: %p", obstacle);
    // }
    // return GetBeatmapDataFromBeatmapSaveData(self, notesSaveData, obstaclesSaveData, eventsSaveData, startBpm, shuffle, shufflePeriod);

    BeatmapData *beatmapData = BeatmapData::New_ctor(4);
    List_1<BeatmapDataLoader::BpmChangeData> *bpmChangesData = List_1<BeatmapDataLoader::BpmChangeData>::New_ctor();
    bpmChangesData->Add(BeatmapDataLoader::BpmChangeData(0, 0, startBpm));
    BeatmapDataLoader::BpmChangeData bpmChangeData = bpmChangesData->get_Item(0);
    for (int i = 0; i < eventsSaveData->get_Count(); i++) {
        BeatmapSaveData::EventData *eventData = eventsSaveData->get_Item(i);
        if (BeatmapDataLoader::ConvertFromBeatmapSaveDataBeatmapEventType(eventData->get_type()) == BeatmapEventType::Event10) {
            float time = eventData->get_time();
            int value = eventData->get_value();
            float bpmChangeStartTime = bpmChangeData.bpmChangeStartTime + self->GetRealTimeFromBPMTime(time - bpmChangeData.bpmChangeStartTime, value, shuffle, shufflePeriod);
            bpmChangesData->Add(BeatmapDataLoader::BpmChangeData(bpmChangeStartTime, time, value));
        }
    }
    notesSaveData->Sort(il2cpp_utils::MakeDelegate<System::Comparison_1<BeatmapSaveData::NoteData*>*>(classof(System::Comparison_1<BeatmapSaveData::NoteData*>*), (void*)nullptr, 
        +[](BeatmapSaveData::NoteData *x, BeatmapSaveData::NoteData *y) {
            if (x->get_time() >= y->get_time()) {
                return 1;
            }
            return -1;
        }
    ));
    waypointsSaveData->Sort(il2cpp_utils::MakeDelegate<System::Comparison_1<BeatmapSaveData::WaypointData*>*>(classof(System::Comparison_1<BeatmapSaveData::WaypointData*>*), (void*)nullptr, 
        +[](BeatmapSaveData::WaypointData *x, BeatmapSaveData::WaypointData *y) {
            if (x->get_time() >= y->get_time()) {
                return 1;
            }
            return -1;
        }
    ));
    obstaclesSaveData->Sort(il2cpp_utils::MakeDelegate<System::Comparison_1<BeatmapSaveData::ObstacleData*>*>(classof(System::Comparison_1<BeatmapSaveData::ObstacleData*>*), (void*)nullptr, 
        +[](BeatmapSaveData::ObstacleData *x, BeatmapSaveData::ObstacleData *y) {
            if (x->get_time() >= y->get_time()) {
                return 1;
            }
            return -1;
        }
    ));
    int notesSaveDataIdx = 0;
    int obstaclesSaveDataIdx = 0;
    int bpmChangesDataIdx = 0;
    while (notesSaveDataIdx < notesSaveData->get_Count() || obstaclesSaveDataIdx < obstaclesSaveData->get_Count()) {
        auto noteData = (notesSaveDataIdx < notesSaveData->get_Count()) ? (CustomJSONData::CustomBeatmapSaveData_NoteData *) notesSaveData->get_Item(notesSaveDataIdx) : nullptr;
        auto waypointData = (notesSaveDataIdx < waypointsSaveData->get_Count()) ? (BeatmapSaveData::WaypointData *) waypointsSaveData->get_Item(notesSaveDataIdx) : nullptr;
        auto obstacleData = (obstaclesSaveDataIdx < obstaclesSaveData->get_Count()) ? (CustomJSONData::CustomBeatmapSaveData_ObstacleData *) obstaclesSaveData->get_Item(obstaclesSaveDataIdx) : nullptr;
        if (noteData && (!obstacleData || noteData->get_time() <= obstacleData->get_time())) {
            float time2 = ProcessTime(self, noteData->get_time(), bpmChangesDataIdx, bpmChangesData, shuffle, shufflePeriod);
            ColorType colorType = BeatmapDataLoader::ConvertFromBeatmapSaveDataNoteType(noteData->type);
            CustomJSONData::CustomNoteData *beatmapObjectData;
            if (colorType == ColorType::None) {
                beatmapObjectData = CustomJSONDataCreateBombNoteData(time2, noteData->lineIndex, noteData->lineLayer);
            } else {
                beatmapObjectData = CustomJSONDataCreateBasicNoteData(time2, noteData->lineIndex, noteData->lineLayer, colorType, noteData->cutDirection);
            }
            beatmapObjectData->customData = noteData->customData;
            beatmapData->AddBeatmapObjectData(beatmapObjectData);
            notesSaveDataIdx++;
        } else if (obstacleData) {
            float obstacleTime = ProcessTime(self, obstacleData->get_time(), bpmChangesDataIdx, bpmChangesData, shuffle, shufflePeriod);
            CustomJSONData::CustomObstacleData *beatmapObjectData2 = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomObstacleData*>(obstacleTime, obstacleData->lineIndex, obstacleData->type, self->GetRealTimeFromBPMTime(obstacleData->duration, startBpm, shuffle, shufflePeriod), obstacleData->width));
            beatmapObjectData2->customData = obstacleData->customData;
            beatmapData->AddBeatmapObjectData(beatmapObjectData2);
            obstaclesSaveDataIdx++;
        }
    }
    for (int i = 0; i < eventsSaveData->get_Count(); i++) {
        auto *eventData = (CustomJSONData::CustomBeatmapSaveData_EventData *) eventsSaveData->get_Item(i);
        float time = eventData->time;
        while (bpmChangesDataIdx < bpmChangesData->get_Count() - 1 && bpmChangesData->get_Item(bpmChangesDataIdx + 1).bpmChangeStartBpmTime < time) {
            bpmChangesDataIdx++;
        }
        BeatmapDataLoader::BpmChangeData bpmChangeData2 = bpmChangesData->get_Item(bpmChangesDataIdx);
        float realTime = bpmChangeData2.bpmChangeStartTime + self->GetRealTimeFromBPMTime(time - bpmChangeData2.bpmChangeStartBpmTime, bpmChangeData2.bpm, shuffle, shufflePeriod);
        BeatmapEventType type = BeatmapDataLoader::ConvertFromBeatmapSaveDataBeatmapEventType(eventData->type);
        CustomJSONData::CustomBeatmapEventData *beatmapEventData = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomBeatmapEventData*>(time, type, eventData->value));
        beatmapEventData->customData = eventData->customData;
        beatmapData->AddBeatmapEventData(beatmapEventData);
    }
    if (beatmapData->beatmapEventsData->get_Count() == 0) {
        beatmapData->AddBeatmapEventData(BeatmapEventData::New_ctor(0, BeatmapEventType::Event0, 1));
        beatmapData->AddBeatmapEventData(BeatmapEventData::New_ctor(0, BeatmapEventType::Event4, 1));
    }
    beatmapData->ProcessRemainingData();
    return beatmapData;
}

void CustomJSONData::InstallHooks() {
    // Install hooks
    INSTALL_HOOK_OFFSETLESS(DeserializeFromJSONString, il2cpp_utils::FindMethodUnsafe("", "BeatmapSaveData", "DeserializeFromJSONString", 1));
    // INSTALL_HOOK_OFFSETLESS(CreateBasicNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBasicNoteData", 5));
    // INSTALL_HOOK_OFFSETLESS(CreateBombNoteData, il2cpp_utils::FindMethodUnsafe("", "NoteData", "CreateBombNoteData", 3));
    // INSTALL_HOOK_OFFSETLESS(AddBeatmapObjectData, il2cpp_utils::FindMethodUnsafe("", "BeatmapData", "AddBeatmapObjectData", 1));
    INSTALL_HOOK_OFFSETLESS(GetBeatmapDataFromBeatmapSaveData, il2cpp_utils::FindMethodUnsafe("", "BeatmapDataLoader", "GetBeatmapDataFromBeatmapSaveData", 8));

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
}