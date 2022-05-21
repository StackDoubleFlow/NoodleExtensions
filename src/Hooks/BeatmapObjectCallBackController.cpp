#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/CallbacksInTime.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/SortedList_1.hpp"
#include "GlobalNamespace/SortedList_2.hpp"
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
            aAheadOfTime = aAheadOfTime - getAD(wrapper).aheadTime;
        }
        if (wrapperB) {
            bAheadOfTime = bAheadOfTime - getAD(wrapperB).aheadTime;
        }

        return aAheadOfTime < bAheadOfTime;
    });


    auto newList = SafePtr(System::Collections::Generic::LinkedList_1<BeatmapDataItem*>::New_ctor());
    auto newListPtr = static_cast<System::Collections::Generic::LinkedList_1<BeatmapDataItem*>*>(newList);
    if (items.empty()) return newListPtr;



    System::Collections::Generic::LinkedListNode_1<BeatmapDataItem *> * prev;

    for (auto const& o : items) {
        newList->AddLast(o);
//        if (!prev) prev = newList->AddFirst(o);
//        else prev = newList->AddAfter(prev, o);
    }

    return newListPtr;
}

MAKE_HOOK_MATCH(BeatmapObjectCallbackController_LateUpdate, &BeatmapCallbacksController::ManualUpdate, void, BeatmapCallbacksController *self, float songTime) {
//    if (controller != self) {
//        self = controller;
//        auto beatmap = il2cpp_utils::cast<CustomJSONData::CustomBeatmapData>(self->beatmapData);
//        il2cpp_utils::cast<GlobalNamespace::SortedList_1<BeatmapDataItem*>>(beatmap->allBeatmapData)->items = SortAndOrderList(beatmap);
//    }

    return BeatmapObjectCallbackController_LateUpdate(self, songTime);

//
//
//    self->songTime = songTime;
//    self->processingCallbacks = true;
//    if (songTime > self->prevSongTime) {
//        auto enumerator = self->callbacksInTimes->GetEnumerator();
//
//        while (enumerator.MoveNext()) {
//            auto keyValuePair = enumerator.get_Current();
//            auto value = keyValuePair.get_Value();
//            for (auto linkedListNode = (value->lastProcessedNode != nullptr)
//                                       ? value->lastProcessedNode->get_Next()
//                                       : self->beatmapData->get_allBeatmapDataItems()->get_First();
//                 linkedListNode != nullptr; linkedListNode = linkedListNode->get_Next()) {
//                auto value2 = linkedListNode->get_Value();
//
//                if (value2->time - value->aheadTime > songTime) {
//                    break;
//                }
//                if (value2->type == BeatmapDataItem::BeatmapDataItemType::BeatmapEvent ||
//                    /// CJD TRANSPILE HERE
//                    value2->type == 2 ||
//                    /// CJD TRANSPILE HERE
//                    (value2->type == BeatmapDataItem::BeatmapDataItemType::BeatmapObject &&
//                     value2->time >= self->startFilterTime)) {
//                    value->CallCallbacks(value2);
//                }
//                value->lastProcessedNode = linkedListNode;
//            }
//        }
//        enumerator.Dispose();
//    } else {
//        auto callbacksInTimesEnumerator = self->callbacksInTimes->GetEnumerator();
//
//        while (callbacksInTimesEnumerator.MoveNext()) {
//            auto keyValuePair2 = callbacksInTimesEnumerator.get_Current();
//            auto value3 = keyValuePair2.get_Value();
//            auto linkedListNode2 = value3->lastProcessedNode;
//            while (linkedListNode2 != nullptr) {
//                auto value4 = linkedListNode2->get_Value();
//                if (value4->time - value3->aheadTime <= songTime) {
//                    break;
//                }
//                if (value4->type == BeatmapDataItem::BeatmapDataItemType::BeatmapEvent) {
//                    auto* beatmapEventData = static_cast<BeatmapEventData *>(value4);
//                    if (beatmapEventData->previousSameTypeEventData != nullptr) {
//                        value3->CallCallbacks(beatmapEventData->previousSameTypeEventData);
//                    } else {
//                        auto def = beatmapEventData->GetDefault(beatmapEventData);
//                        if (def != nullptr) {
//                            value3->CallCallbacks(def);
//                        }
//                    }
//                    linkedListNode2 = linkedListNode2->get_Previous();
//                    value3->lastProcessedNode = linkedListNode2;
//                }
//            }
//        }
//    }
//
//    finish:
//    self->prevSongTime = songTime;
//    self->processingCallbacks = false;
}


void InstallBeatmapObjectCallbackControllerHooks(Logger& logger) {
    INSTALL_HOOK(logger, BeatmapObjectCallbackController_LateUpdate);
}

NEInstallHooks(InstallBeatmapObjectCallbackControllerHooks);