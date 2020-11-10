#pragma once

#include "custom-types/shared/macros.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "System/Object.hpp"

#include "NELogger.h"

using namespace GlobalNamespace;

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapData, BeatmapData,
    DECLARE_CTOR(ctor);

    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    REGISTER_FUNCTION(CustomBeatmapData,
        NELogger::GetLogger().debug("Registering CustomBeatmapData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomBeatmapEventData, BeatmapEventData,
    DECLARE_CTOR(ctor);

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


DECLARE_CLASS_CODEGEN(CustomJSONData, CustomObstacleData, ObstacleData,
    DECLARE_CTOR(ctor);

    DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    REGISTER_FUNCTION(CustomObstacleData,
        NELogger::GetLogger().debug("Registering CustomObstacleData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        REGISTER_METHOD(Finalize);
    )

public:
    rapidjson::Value *customData;
)

DECLARE_CLASS_CODEGEN(CustomJSONData, CustomNoteData, NoteData,
    DECLARE_CTOR(ctor, float time, int lineIndex, NoteLineLayer noteLineLayer, NoteLineLayer startNoteLineLayer, ColorType colorType, NoteCutDirection cutDirection, float timeToNextColorNote, float timeToPrevColorNote, int flipLineIndex, float flipYSide, float duration);

    // DECLARE_METHOD(static CustomNoteData*, CreateBasicNoteData, float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection);
    // DECLARE_METHOD(static CustomNoteData*, CreateBombNoteData, float time, int lineIndex, NoteLineLayer noteLineLayer);

    // DECLARE_OVERRIDE_METHOD(void, Finalize, il2cpp_utils::FindMethod("System", "Object", "Finalize"));

    REGISTER_FUNCTION(CustomNoteData,
        NELogger::GetLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);

        // Register methods
        // REGISTER_METHOD(CreateBasicNoteData);
        // REGISTER_METHOD(CreateBombNoteData);
        // REGISTER_METHOD(Finalize);
    )

public:
    rapidjson::Value *customData;
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