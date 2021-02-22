
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "System/Collections/Generic/List_1.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"

#include <optional>

template<class T>
using List = System::Collections::Generic::List_1<T>;

using namespace GlobalNamespace;

MAKE_HOOK_OFFSETLESS(GetBeatmapDataFromBeatmapSaveData, BeatmapData *, BeatmapDataLoader *self, List<BeatmapSaveData::NoteData *> *notesSaveData, List<BeatmapSaveData::WaypointData *> *waypointsSaveData, 
    List<BeatmapSaveData::ObstacleData *> *obstaclesSaveData, List<BeatmapSaveData::EventData *> *eventsSaveData, BeatmapSaveData::SpecialEventKeywordFiltersData *evironmentSpecialEventFilterData, float startBpm, float shuffle, float shufflePeriod) {
    auto *result = (CustomJSONData::CustomBeatmapData *) GetBeatmapDataFromBeatmapSaveData(self, notesSaveData, waypointsSaveData, obstaclesSaveData, eventsSaveData, evironmentSpecialEventFilterData, startBpm, shuffle, shufflePeriod);
    
    

    return result;
}


void NoodleExtensions::InstallBeatmapDataLoaderHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, GetBeatmapDataFromBeatmapSaveData, il2cpp_utils::FindMethodUnsafe("", "BeatmapDataLoader", "GetBeatmapDataFromBeatmapSaveData", 8));
}