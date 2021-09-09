
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"

#include "Animation/AnimationHelper.h"
#include "tracks/shared/Animation/Track.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include <optional>

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(GetBeatmapDataFromBeatmapSaveData,
                &BeatmapDataLoader::GetBeatmapDataFromBeatmapSaveData,
                BeatmapData *, BeatmapDataLoader *self,
                List<BeatmapSaveData::NoteData *> *notesSaveData,
                List<BeatmapSaveData::WaypointData *> *waypointsSaveData,
                List<BeatmapSaveData::ObstacleData *> *obstaclesSaveData,
                List<BeatmapSaveData::EventData *> *eventsSaveData,
                BeatmapSaveData::SpecialEventKeywordFiltersData
                    *evironmentSpecialEventFilterData,
                float startBpm, float shuffle, float shufflePeriod) {
    // This should call CJD's hook
    auto *result =
        (CustomJSONData::CustomBeatmapData *)GetBeatmapDataFromBeatmapSaveData(
            self, notesSaveData, waypointsSaveData, obstaclesSaveData,
            eventsSaveData, evironmentSpecialEventFilterData, startBpm, shuffle,
            shufflePeriod);
    NELogger::GetLogger().info("BeatmapData klass name is %s",
                               result->klass->name);

    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    auto &beatmapAD = TracksAD::getBeatmapAD(result->customData);

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(result);
    }

    for (int i = 0; i < result->beatmapLinesData->Length(); i++) {
        BeatmapLineData *beatmapLineData = result->beatmapLinesData->values[i];
        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData =
                beatmapLineData->beatmapObjectsData->items->values[j];

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto obstacleData =
                    (CustomJSONData::CustomObstacleData *)beatmapObjectData;
                customDataWrapper = obstacleData->customData;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData =
                    (CustomJSONData::CustomNoteData *)beatmapObjectData;
                customDataWrapper = noteData->customData;
            } else {
                continue;
            }

            if (customDataWrapper->value) {
                rapidjson::Value &customData = *customDataWrapper->value;
                BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);
                ad.objectData = ObjectCustomData(customData);

                rapidjson::Value &animation = customData["_animation"];
                ad.animationData = AnimationObjectData(beatmapAD, animation);
            }
        }
    }

    return result;
}

void InstallBeatmapDataLoaderHooks(Logger &logger) {
    INSTALL_HOOK(logger, GetBeatmapDataFromBeatmapSaveData);
}

NEInstallHooks(InstallBeatmapDataLoaderHooks);