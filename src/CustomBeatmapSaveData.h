#pragma once

#include "custom-types/shared/macros.hpp"

#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"

using namespace GlobalNamespace;

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_NoteData, BeatmapSaveData::NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, NoteLineLayer lineLayer, BeatmapSaveData::NoteType type, NoteCutDirection cutDirection);

    REGISTER_FUNCTION(CustomBeatmapSaveData_NoteData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_ObstacleData, BeatmapSaveData::ObstacleData,
    DECLARE_CTOR(ctor, float time, int lineIndex, ObstacleType type, float duration, int width);

    REGISTER_FUNCTION(CustomBeatmapSaveData_ObstacleData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapSaveData_EventData, BeatmapSaveData::EventData,
    DECLARE_CTOR(ctor, float time, BeatmapSaveData::BeatmapEventType type, int value);

    REGISTER_FUNCTION(CustomBeatmapSaveData_EventData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)



