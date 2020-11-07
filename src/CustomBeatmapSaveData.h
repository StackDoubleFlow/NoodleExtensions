#pragma once

#include "custom-types/shared/macros.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "System/Collections/Generic/List_1.hpp"

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData, BeatmapSaveData,
    DECLARE_CTOR(ctor, 
                 List_1<BeatmapSaveData::EventData*>* events, 
                 List_1<BeatmapSaveData::NoteData*>* notes, 
                 List_1<BeatmapSaveData::ObstacleData*>* obstacles);

    REGISTER_FUNCTION(CustomBeatmapSaveData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_NoteData, BeatmapSaveData::NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, NoteLineLayer lineLayer, BeatmapSaveData::NoteType type, NoteCutDirection cutDirection);

    REGISTER_FUNCTION(CustomBeatmapSaveData_NoteData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_ObstacleData, BeatmapSaveData::ObstacleData,
    DECLARE_CTOR(ctor, float time, int lineIndex, ObstacleType type, float duration, int width);

    REGISTER_FUNCTION(CustomBeatmapSaveData_ObstacleData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_EventData, BeatmapSaveData::EventData,
    DECLARE_CTOR(ctor, float time, BeatmapSaveData::BeatmapEventType type, int value);

    REGISTER_FUNCTION(CustomBeatmapSaveData_EventData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )

public:
    rapidjson::Value *customData;
)



