#pragma once

#include "custom-types/shared/macros.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "GlobalNamespace/BeatmapSaveData_WaypointData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventKeywordFiltersData.hpp"
#include "System/Collections/Generic/List_1.hpp"

#include "NELogger.h"

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData, BeatmapSaveData,
    DECLARE_CTOR(ctor, 
                 List_1<BeatmapSaveData::EventData*>* events, 
                 List_1<BeatmapSaveData::NoteData*>* notes, 
                 List_1<BeatmapSaveData::WaypointData*>* waypoints, 
                 List_1<BeatmapSaveData::ObstacleData*>* obstacles,
                 BeatmapSaveData::SpecialEventKeywordFiltersData *specialEventsKeywordFilters);

    REGISTER_FUNCTION(CustomBeatmapSaveData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_NoteData, BeatmapSaveData::NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, NoteLineLayer lineLayer, BeatmapSaveData::NoteType type, NoteCutDirection cutDirection);

    REGISTER_FUNCTION(CustomBeatmapSaveData_NoteData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_NoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_ObstacleData, BeatmapSaveData::ObstacleData,
    DECLARE_CTOR(ctor, float time, int lineIndex, ObstacleType type, float duration, int width);

    REGISTER_FUNCTION(CustomBeatmapSaveData_ObstacleData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_ObstacleData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapSaveData_EventData, BeatmapSaveData::EventData,
    DECLARE_CTOR(ctor, float time, BeatmapSaveData::BeatmapEventType type, int value);

    REGISTER_FUNCTION(CustomBeatmapSaveData_EventData,
        NELogger::GetLogger().debug("Registering CustomBeatmapSaveData_EventData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)



