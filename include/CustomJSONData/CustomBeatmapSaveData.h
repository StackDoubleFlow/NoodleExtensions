#pragma once

#include "custom-types/shared/macros.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "GlobalNamespace/BeatmapSaveData_WaypointData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventKeywordFiltersData.hpp"

#include "NELogger.h"

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData, GlobalNamespace::BeatmapSaveData,
    DECLARE_CTOR(ctor, 
                 List<GlobalNamespace::BeatmapSaveData::EventData*>* events, 
                 List<GlobalNamespace::BeatmapSaveData::NoteData*>* notes, 
                 List<GlobalNamespace::BeatmapSaveData::WaypointData*>* waypoints, 
                 List<GlobalNamespace::BeatmapSaveData::ObstacleData*>* obstacles,
                 BeatmapSaveData::SpecialEventKeywordFiltersData *specialEventsKeywordFilters);

    REGISTER_FUNCTION(CustomBeatmapSaveData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_NoteData, GlobalNamespace::BeatmapSaveData::NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, GlobalNamespace::NoteLineLayer lineLayer, GlobalNamespace::BeatmapSaveData::NoteType type, GlobalNamespace::NoteCutDirection cutDirection);

    REGISTER_FUNCTION(CustomBeatmapSaveData_NoteData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_NoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_ObstacleData, GlobalNamespace::BeatmapSaveData::ObstacleData,
    DECLARE_CTOR(ctor, float time, int lineIndex, GlobalNamespace::ObstacleType type, float duration, int width);

    REGISTER_FUNCTION(CustomBeatmapSaveData_ObstacleData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_ObstacleData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_EventData, GlobalNamespace::BeatmapSaveData::EventData,
    DECLARE_CTOR(ctor, float time, GlobalNamespace::BeatmapSaveData::BeatmapEventType type, int value);

    REGISTER_FUNCTION(CustomBeatmapSaveData_EventData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_EventData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

