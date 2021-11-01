#pragma once

#include "custom-json-data/shared/JSONWrapper.h"

namespace GlobalNamespace {
    class IDifficultyBeatmap;
}

namespace CustomJSONData {
    class CustomBeatmapData;
}

namespace NoodleExtensions {
    class SceneTransitionHelper {
    public:
        static bool CheckIfInArray(ValueUTF16 const& val, std::u16string_view stringToCheck);

        static void Patch(GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap, CustomJSONData::CustomBeatmapData * customBeatmapDataCustom);
    };
}
