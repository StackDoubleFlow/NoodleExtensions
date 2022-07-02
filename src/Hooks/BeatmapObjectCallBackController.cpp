#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/CallbacksInTime.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/SortedList_1.hpp"
#include "GlobalNamespace/SortedList_2.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Action.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/Events.h"
#include "AssociatedData.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "SharedUpdate.h"

using namespace GlobalNamespace;

BeatmapCallbacksController* controller;
static GlobalNamespace::IReadonlyBeatmapData* beatmapData;

float ObjectSortGetTime(BeatmapDataItem const* n) {
    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    if (n->klass == customObstacleDataClass) {
        auto *obstacle = reinterpret_cast<CustomJSONData::CustomObstacleData const*>(n);
        return n->time - obstacle->aheadTimeNoodle;
    } else if (n->klass == customNoteDataClass) {
        auto *note = reinterpret_cast<CustomJSONData::CustomNoteData const*>(n);
        return n->time - note->aheadTimeNoodle;
    } else {
        return n->time;
    }
}

constexpr bool ObjectTimeCompare(BeatmapDataItem const * a, BeatmapDataItem const* b) {
    return ObjectSortGetTime(a) < ObjectSortGetTime(b);
}

System::Collections::Generic::LinkedList_1<BeatmapDataItem*>* SortAndOrderList(CustomJSONData::CustomBeatmapData* beatmapData) {
    auto items = beatmapData->GetAllBeatmapItemsCpp();

    std::stable_sort(items.begin(), items.end(), ObjectTimeCompare);


    auto newList = SafePtr(System::Collections::Generic::LinkedList_1<BeatmapDataItem*>::New_ctor());
    auto newListPtr = static_cast<System::Collections::Generic::LinkedList_1<BeatmapDataItem*>*>(newList);
    if (items.empty()) return newListPtr;



    for (auto const& o : items) {
        newList->AddLast(o);
    }

    return newListPtr;
}

MAKE_HOOK_MATCH(BeatmapCallbacksUpdater_LateUpdate, &BeatmapCallbacksUpdater::LateUpdate, void, BeatmapCallbacksUpdater *self) {
    auto selfController = self->beatmapCallbacksController;

    // Reset to avoid overriding non NE maps
//    if ((controller || beatmapData) && (controller != selfController || selfController->beatmapData != beatmapData)) {
//        CustomJSONData::CustomEventCallbacks::firstNode.emplace(nullptr);
//    }

    if (!Hooks::isNoodleHookEnabled()) {
        controller = nullptr;
        beatmapData = nullptr;
        return BeatmapCallbacksUpdater_LateUpdate(self);
    }

    if (controller != selfController || selfController->beatmapData != beatmapData) {
        CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Using noodle sorted node");
        controller = selfController;
        beatmapData = selfController->beatmapData;


        auto beatmap = il2cpp_utils::cast<CustomJSONData::CustomBeatmapData>(selfController->beatmapData);
        auto items = SortAndOrderList(beatmap);

        auto first = items->get_First();
        CustomJSONData::CustomEventCallbacks::firstNode.emplace(first);
    }

    return BeatmapCallbacksUpdater_LateUpdate(self);
}


void InstallBeatmapObjectCallbackControllerHooks(Logger& logger) {
    INSTALL_HOOK(logger, BeatmapCallbacksUpdater_LateUpdate);
}

NEInstallHooks(InstallBeatmapObjectCallbackControllerHooks);