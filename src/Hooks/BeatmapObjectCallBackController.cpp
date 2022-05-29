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


static CustomJSONData::JSONWrapper* customData(Il2CppObject* obj) {
    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    CustomJSONData::JSONWrapper *customDataWrapper = nullptr;
    float bpm;
    if (obj->klass == customObstacleDataClass) {
        auto *obstacleData = (CustomJSONData::CustomObstacleData *) obj;
        customDataWrapper = obstacleData->customData;
        bpm = obstacleData->bpm;
    } else if (obj->klass == customNoteDataClass) {
        auto *noteData = (CustomJSONData::CustomNoteData *) obj;
        customDataWrapper = noteData->customData;
        bpm = noteData->bpm;
    }

    return customDataWrapper;
}

System::Collections::Generic::LinkedList_1<BeatmapDataItem*>* SortAndOrderList(CustomJSONData::CustomBeatmapData* beatmapData) {
    auto items = beatmapData->GetAllBeatmapItemsCpp();

    std::stable_sort(items.begin(), items.end(), [](auto const& a, auto const& b ) {
        float aAheadOfTime = a->time;
        float bAheadOfTime = b->time;

        auto wrapper = customData(a);
        auto wrapperB = customData(b);

        if (wrapper) {
            aAheadOfTime = aAheadOfTime - *getAD(wrapper).aheadTime;
        }
        if (wrapperB) {
            bAheadOfTime = bAheadOfTime - *getAD(wrapperB).aheadTime;
        }

        return aAheadOfTime < bAheadOfTime;
    });


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
    if ((controller || beatmapData) && (controller != selfController || selfController->beatmapData != beatmapData)) {
        CustomJSONData::CustomEventCallbacks::firstNode.emplace(nullptr);
    }

    if (!Hooks::isNoodleHookEnabled()) {
        controller = nullptr;
        beatmapData = nullptr;
        return BeatmapCallbacksUpdater_LateUpdate(self);
    }

    if (controller != selfController || selfController->beatmapData != beatmapData) {
        controller = selfController;
        beatmapData = selfController->beatmapData;

        // To force CJD to register the callback
        // if self->prevTime == songTime
        // early return is called by CJD
        // TODO: Define this behaviour
//        BeatmapObjectCallbackController_LateUpdate(self, self->prevSongTime);


        auto beatmap = il2cpp_utils::cast<CustomJSONData::CustomBeatmapData>(selfController->beatmapData);
        auto items = SortAndOrderList(beatmap);

        auto first = items->get_First();
        CustomJSONData::CustomEventCallbacks::firstNode.emplace(first);

//        auto enumerator = self->callbacksInTimes->GetEnumerator();
//        while (enumerator.MoveNext()) {
//            auto keyValuePair = enumerator.get_Current();
//            auto value = keyValuePair.get_Value();
//
//            value->lastProcessedNode = first;
//        }
//        enumerator.Dispose();

//        il2cpp_utils::cast<GlobalNamespace::SortedList_1<BeatmapDataItem*>>(beatmap->allBeatmapData)->items = SortAndOrderList(beatmap);
    }

    return BeatmapCallbacksUpdater_LateUpdate(self);
}


void InstallBeatmapObjectCallbackControllerHooks(Logger& logger) {
    INSTALL_HOOK(logger, BeatmapCallbacksUpdater_LateUpdate);
}

NEInstallHooks(InstallBeatmapObjectCallbackControllerHooks);