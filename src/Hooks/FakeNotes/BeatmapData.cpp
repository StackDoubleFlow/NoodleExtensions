#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapSaveData.hpp"
#include "GlobalNamespace/BeatmapSaveData_NoteData.hpp"
#include "GlobalNamespace/BeatmapSaveData_ObstacleData.hpp"
#include "GlobalNamespace/BeatmapSaveData_WaypointData.hpp"
#include "GlobalNamespace/BeatmapSaveData_EventData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventKeywordFiltersData.hpp"
#include "GlobalNamespace/BeatmapSaveData_SpecialEventsForKeyword.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "pinkcore/shared/API.hpp"

#include "FakeNoteHelper.h"
#include "SceneTransitionHelper.hpp"
#include "NEHooks.h"
#include "NEJSON.h"

#include "songloader/shared/API.hpp"


typedef rapidjson::GenericDocument<rapidjson::UTF16<char16_t>> DocumentUTF16;

using namespace System;
using namespace System::Collections::Generic;
using namespace GlobalNamespace;
using namespace CustomJSONData;

struct BeatmapRemoveData {
    int toRemoveObstacle = 0;
    int toRemoveNote = 0;
    int toRemoveBomb = 0;
};

static std::unordered_map<BeatmapData*, BeatmapRemoveData> beatmapRemoveDatas;

// return true if fake
// subtracts from object count if fake
static bool FakeObjectRemove(BeatmapData *self, BeatmapObjectData* beatmapObjectData)
{
    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    int* countField = nullptr;
    CustomJSONData::JSONWrapper *customDataWrapper = nullptr;

    if (beatmapObjectData->klass == customObstacleDataClass) {
        auto obstacleData =
                (CustomJSONData::CustomObstacleData *)beatmapObjectData;
        customDataWrapper = obstacleData->customData;
        countField = &self->obstaclesCount;
    } else if (beatmapObjectData->klass == customNoteDataClass) {
        auto noteData = (CustomJSONData::CustomNoteData *)beatmapObjectData;
        customDataWrapper = noteData->customData;

        if (noteData->cutDirection == NoteCutDirection::None)
        {
            // bomb
            countField = &self->bombsCount;
        } else {
            countField = &self->cuttableNotesCount;
        }
    }

    if (customDataWrapper && customDataWrapper->value) {
        rapidjson::Value const& customData = *customDataWrapper->value;

        auto fake = NEJSON::ReadOptionalBool(customData, "_fake");

        if (fake.value_or(false)) {
            int& countRef = *countField;
            countRef--;

            return true;
        }
    }

    return false;
}


static void OnBeatmapLoad(GlobalNamespace::StandardLevelInfoSaveData* infoDat, const std::string& filepath, GlobalNamespace::BeatmapData* beatmapData) {
    auto it = beatmapRemoveDatas.find(beatmapData);

    if (it != beatmapRemoveDatas.end()) {
        auto const &data = it->second;

        bool isNoodle = false;
        if (auto customBeatmap = il2cpp_utils::try_cast<CustomBeatmapData>(beatmapData)) {
            if (customBeatmap.value()->levelCustomData) {
                auto customData = customBeatmap.value()->levelCustomData->value;

                if (customData) {
                    isNoodle = NoodleExtensions::SceneTransitionHelper::CheckIfNoodle(*customData);
                }
            }
        }


        if (isNoodle) {
            beatmapData->bombsCount -= data.toRemoveBomb;
            beatmapData->cuttableNotesCount -= data.toRemoveNote;
            beatmapData->obstaclesCount -= data.toRemoveObstacle;
        }

        beatmapRemoveDatas.erase(it);
    }
}

MAKE_HOOK_MATCH(BeatmapData_AddBeatmapObjectData_t, &BeatmapData::AddBeatmapObjectData, void,
               BeatmapData *self, BeatmapObjectData* beatmapObjectData) {
    BeatmapData_AddBeatmapObjectData_t(self, beatmapObjectData);

    FakeObjectRemove(self, beatmapObjectData);
}

void InstallBeatmapDataHooks(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapData_AddBeatmapObjectData_t);

    // force CJD to be first
    Modloader::requireMod("CustomJSONData");
//    RuntimeSongLoader::API::AddBeatmapDataLoadedEvent(&OnBeatmapLoad);

}
NEInstallHooks(InstallBeatmapDataHooks);