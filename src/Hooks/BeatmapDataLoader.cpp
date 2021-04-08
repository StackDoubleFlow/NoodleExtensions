
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "System/Collections/Generic/List_1.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/Track.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"

#include <optional>

template<class T>
using List = System::Collections::Generic::List_1<T>;

using namespace GlobalNamespace;

Il2CppClass *customObstacleDataClass;
Il2CppClass *customNoteDataClass;

MAKE_HOOK_OFFSETLESS(GetBeatmapDataFromBeatmapSaveData, BeatmapData*, BeatmapDataLoader *self, 
                     List<BeatmapSaveData::NoteData*> *notesSaveData, 
                     List<BeatmapSaveData::WaypointData*> *waypointsSaveData, 
                     List<BeatmapSaveData::ObstacleData *> *obstaclesSaveData, 
                     List<BeatmapSaveData::EventData*> *eventsSaveData, 
                     BeatmapSaveData::SpecialEventKeywordFiltersData *evironmentSpecialEventFilterData, 
                     float startBpm, float shuffle, float shufflePeriod) {
    // This should call CJD's hook
    auto *result = (CustomJSONData::CustomBeatmapData *) GetBeatmapDataFromBeatmapSaveData(self, notesSaveData, waypointsSaveData, obstaclesSaveData, eventsSaveData, evironmentSpecialEventFilterData, startBpm, shuffle, shufflePeriod);
    NELogger::GetLogger().info("BeatmapData klass name is %s", result->klass->name);

    if (!customObstacleDataClass) {
        customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
        customNoteDataClass = classof(CustomJSONData::CustomNoteData *);
    }

    BeatmapAssociatedData *beatmapAD = new BeatmapAssociatedData();
    result->customData->associatedData['N'] = beatmapAD;
    
    auto& tracks = beatmapAD->tracks;

    for (int i = 0; i < result->beatmapLinesData->Length(); i++) {
        BeatmapLineData *beatmapLineData = result->beatmapLinesData->values[i]; 
        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData = beatmapLineData->beatmapObjectsData->items->values[j];

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto obstacleData = (CustomJSONData::CustomObstacleData *) beatmapObjectData;
                customDataWrapper = obstacleData->customData;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
                customDataWrapper = noteData->customData;
            } else {
                continue;
            }

            if (customDataWrapper->value) {
                rapidjson::Value &customData = *customDataWrapper->value;
                if (customData.HasMember("_track")) {
                    std::string trackName = customData["_track"].GetString();
                    Track *track = &tracks[trackName];
                    AnimationHelper::OnTrackCreated(track);
                    getAD(customDataWrapper)->track = track;
                }
            }
        }
    }

    if (!result->customData->value) {
        return result;
    }
    rapidjson::Value &customData = *result->customData->value;

    PointDefinitionManager pointDataManager;
    if (result->customData->value->HasMember("_pointDefinitions")) {
        const rapidjson::Value& pointDefinitions = customData["_pointDefinitions"]; 
        for (rapidjson::Value::ConstValueIterator itr = pointDefinitions.Begin(); itr != pointDefinitions.End(); itr++) {
            std::string pointName = (*itr)["_name"].GetString();
            PointDefinition pointData((*itr)["_points"]);
            pointDataManager.AddPoint(pointName, pointData);
        }
    }
    beatmapAD->pointDefinitions = pointDataManager.pointData;

    return result;
}

MAKE_HOOK_OFFSETLESS(Assert_IsTrue, void, bool condition, Il2CppString *message, Array<Il2CppObject> *args) {}

void NoodleExtensions::InstallBeatmapDataLoaderHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, GetBeatmapDataFromBeatmapSaveData, il2cpp_utils::FindMethodUnsafe("", "BeatmapDataLoader", "GetBeatmapDataFromBeatmapSaveData", 8));
    INSTALL_HOOK_ORIG(logger, Assert_IsTrue, il2cpp_utils::FindMethodUnsafe("NUnit.Framework", "Assert", "IsTrue", 3));
}
