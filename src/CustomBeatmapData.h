#pragma once

#include "custom-types/shared/macros.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/NoteData.hpp"

using namespace GlobalNamespace;

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapData, BeatmapData,
    DECLARE_CTOR(ctor);

    REGISTER_FUNCTION(CustomBeatmapData,
        // getLogger().debug("Registering CustomBeatmapData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomBeatmapEventData, BeatmapEventData,
    DECLARE_CTOR(ctor);

    REGISTER_FUNCTION(CustomBeatmapEventData,
        // getLogger().debug("Registering CustomBeatmapEventData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)


DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomObstacleData, ObstacleData,
    DECLARE_CTOR(ctor);

    REGISTER_FUNCTION(CustomObstacleData,
        // getLogger().debug("Registering CustomObstacleData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)

DECLARE_CLASS_CODEGEN(Il2CppNamespace, CustomNoteData, NoteData,
    DECLARE_CTOR(ctor);

    REGISTER_FUNCTION(CustomNoteData,
        // getLogger().debug("Registering CustomNoteData!");

        // Register constructor
        REGISTER_METHOD(ctor);
    )
)
