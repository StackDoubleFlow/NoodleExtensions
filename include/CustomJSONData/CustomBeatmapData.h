#pragma once

#include "custom-types/shared/macros.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/WaypointData.hpp"
#include "System/Object.hpp"

#include "CustomJSONData/JSONWrapper.h"
#include "NELogger.h"

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapData, GlobalNamespace::BeatmapData,
    DECLARE_CTOR(ctor);

    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));
    DECLARE_OVERRIDE_METHOD(BeatmapData *, GetCopy, il2cpp_utils::FindMethod("", "BeatmapData", "GetCopy"));

    DECLARE_INSTANCE_FIELD(CustomJSONData::JSONWrapper *, customData);

    REGISTER_FUNCTION(CustomBeatmapData,
        NELogger::GetLogger().debug("Registering CustomBeatmapData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);

        // Register fields
        REGISTER_FIELD(customData);
    )
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapEventData, GlobalNamespace::BeatmapEventData,
    DECLARE_CTOR(ctor, float time, GlobalNamespace::BeatmapEventType type, int value);

    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    REGISTER_FUNCTION(CustomBeatmapEventData,
        NELogger::GetLogger().debug("Registering CustomBeatmapEventData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
    )

public:
    rapidjson::Value *customData;
)


DECLARE_CLASS_CODEGEN(CustomJSONData, CustomObstacleData, GlobalNamespace::ObstacleData,
    DECLARE_CTOR(ctor, float time, int lineIndex, GlobalNamespace::ObstacleType obstacleType, float duration, int width);

    DECLARE_OVERRIDE_METHOD(BeatmapObjectData *, GetCopy, il2cpp_utils::FindMethod("", "BeatmapObjectData", "GetCopy"));
    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    DECLARE_INSTANCE_FIELD(CustomJSONData::JSONWrapper *, customData);
    // Used for Noodle Extensions
    DECLARE_INSTANCE_FIELD(float, bpm);
    DECLARE_INSTANCE_FIELD(float, aheadTime);

    REGISTER_FUNCTION(CustomObstacleData,
        NELogger::GetLogger().debug("Registering CustomObstacleData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(GetCopy);
        REGISTER_METHOD(Finalize);

        // Register fields
        REGISTER_FIELD(customData);
        REGISTER_FIELD(bpm);
        REGISTER_FIELD(aheadTime);    
    )
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomNoteData, GlobalNamespace::NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, GlobalNamespace::NoteLineLayer noteLineLayer, GlobalNamespace::NoteLineLayer startNoteLineLayer, GlobalNamespace::ColorType colorType, GlobalNamespace::NoteCutDirection cutDirection, float timeToNextColorNote, float timeToPrevColorNote, int flipLineIndex, float flipYSide, float duration);

    DECLARE_OVERRIDE_METHOD(BeatmapObjectData *, GetCopy, il2cpp_utils::FindMethod("", "BeatmapObjectData", "GetCopy"));
    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    DECLARE_INSTANCE_FIELD(CustomJSONData::JSONWrapper *, customData);
    // Used for Noodle Extensions
    DECLARE_INSTANCE_FIELD(float, bpm);
    DECLARE_INSTANCE_FIELD(float, aheadTime);

    REGISTER_FUNCTION(CustomNoteData,
        NELogger::GetLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
        REGISTER_METHOD(GetCopy);

        // Register fields
        REGISTER_FIELD(customData);
        REGISTER_FIELD(bpm);
        REGISTER_FIELD(aheadTime);
    )
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomWaypointData, GlobalNamespace::WaypointData,
    DECLARE_CTOR(ctor, float time, int lineIndex, GlobalNamespace::NoteLineLayer noteLineLayer, GlobalNamespace::OffsetDirection offsetDirection);

    DECLARE_OVERRIDE_METHOD(BeatmapObjectData *, GetCopy, il2cpp_utils::FindMethod("", "BeatmapObjectData", "GetCopy"));
    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    // Used for Noodle Extensions
    DECLARE_INSTANCE_FIELD(float, bpm);
    DECLARE_INSTANCE_FIELD(float, aheadTime);

    REGISTER_FUNCTION(CustomNoteData,
        NELogger::GetLogger().debug("Registering CustomWaypointData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
        REGISTER_METHOD(GetCopy);

        // Register fields
        REGISTER_FIELD(bpm);
        REGISTER_FIELD(aheadTime);
    )
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomEventData, System::Object, 
    DECLARE_CTOR(ctor, Il2CppString* type, float time);

    DECLARE_INSTANCE_FIELD(Il2CppString*, type);
    DECLARE_INSTANCE_FIELD(float, time);

    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    REGISTER_FUNCTION(CustomEventData,
        NELogger::GetLogger().debug("Registering CustomEventData!");

        // Register fields
        REGISTER_FIELD(type);
        REGISTER_FIELD(time);

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
    )

public:
    rapidjson::Value *data;
)