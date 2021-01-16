#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapObjectCallbackController_BeatmapObjectCallbackData.hpp"
#include "GlobalNamespace/BeatmapObjectCallbackController_BeatmapEventCallbackData.hpp"
#include "GlobalNamespace/BeatmapObjectCallbackController_BeatmapEventCallback.hpp"
#include "GlobalNamespace/BeatmapObjectCallbackController_BeatmapObjectCallback.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Action.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"

template<class T>
using List = System::Collections::Generic::List_1<T>;

using namespace GlobalNamespace;

// BeatmapDataTransformHelper.cpp
extern Il2CppClass *customObstacleDataClass;
extern Il2CppClass *customNoteDataClass;

MAKE_HOOK_OFFSETLESS(LateUpdate, void, BeatmapObjectCallbackController *self) {
    if (!self->beatmapData) {
        return;
    }

    auto *beatmapLinesData = reinterpret_cast<Array<BeatmapLineData *> *>(self->beatmapData->get_beatmapLinesData());

    for (int i = 0; i < self->beatmapObjectCallbackData->size; i++) {
        self->beatmapObjectDataCallbackCacheList->Clear();
        BeatmapObjectCallbackController::BeatmapObjectCallbackData *callbackData = self->beatmapObjectCallbackData->items->values[i];
        for (int j = 0; j < beatmapLinesData->Length(); j++) {
            while (callbackData->nextObjectIndexInLine->values[j] < ((List<GlobalNamespace::BeatmapObjectData*>*) beatmapLinesData->values[j]->get_beatmapObjectsData())->get_Count()) {
                BeatmapObjectData *beatmapObjectData = beatmapLinesData->values[j]->get_beatmapObjectsData()->System_Collections_Generic_IReadOnlyList_1_get_Item(callbackData->nextObjectIndexInLine->values[j]);

                float aheadTime = callbackData->aheadTime;
                if (callbackData->callback->method_ptr.m_value == il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnController", "HandleBeatmapObjectCallback", 1)->methodPointer) {
                    if (beatmapObjectData->klass == customObstacleDataClass) {
                        auto obstacleData = (CustomJSONData::CustomObstacleData *) beatmapObjectData;
                        aheadTime = getAD(obstacleData->customData)->aheadTime;
                    } else if (beatmapObjectData->klass == customNoteDataClass) {
                        auto noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
                        // NELogger::GetLogger().info("noteData aheadTime: %f", noteData->aheadTime);
                        aheadTime = getAD(noteData->customData)->aheadTime;
                    }
                }
                // NELogger::GetLogger().info("Method name: %s", callbackData->callback->method_info->get_Name());
                

                if (beatmapObjectData->time - aheadTime >= self->audioTimeSource->get_songTime()) {
                    break;
                }
                if (beatmapObjectData->time >= self->spawningStartTime) {
                    for (int k = self->beatmapObjectDataCallbackCacheList->get_Count(); k >= 0; k--) {
                        if (k == 0 || self->beatmapObjectDataCallbackCacheList->items->values[k - 1]->time <= beatmapObjectData->time) {
                            self->beatmapObjectDataCallbackCacheList->Insert(k, beatmapObjectData);
                            break;
                        }
                    }
                }
                callbackData->nextObjectIndexInLine->values[j]++;
            }
        }
        for (int j = 0; j < self->beatmapObjectDataCallbackCacheList->size; j++) {
            callbackData->callback->Invoke(self->beatmapObjectDataCallbackCacheList->items->values[j]);
        }
    }

    for (int l = 0; l < self->beatmapEventCallbackData->get_Count(); l++) {
        BeatmapObjectCallbackController::BeatmapEventCallbackData *callbackData = self->beatmapEventCallbackData->items->values[l];
        while (callbackData->nextEventIndex < ((List<GlobalNamespace::BeatmapEventData*>*) self->beatmapData->get_beatmapEventsData())->get_Count()) {
            BeatmapEventData *beatmapEventData = self->beatmapData->get_beatmapEventsData()->System_Collections_Generic_IReadOnlyList_1_get_Item(callbackData->nextEventIndex);
            if (beatmapEventData->time - callbackData->aheadTime >= self->audioTimeSource->get_songTime()) {
                break;
            }
            if (self->validEvents->Contains(beatmapEventData->type)) {
                callbackData->callback->Invoke(beatmapEventData);
            }
            callbackData->nextEventIndex++;
        }
    }
    while (self->nextEventIndex < ((List<GlobalNamespace::BeatmapEventData*>*) self->beatmapData->get_beatmapEventsData())->get_Count()) {
        BeatmapEventData *beatmapEventData = self->beatmapData->get_beatmapEventsData()->System_Collections_Generic_IReadOnlyList_1_get_Item(self->nextEventIndex);
        if (beatmapEventData->time >= self->audioTimeSource->get_songTime()) {
            break;
        }
        self->SendBeatmapEventDidTriggerEvent(beatmapEventData);
        self->nextEventIndex++;
    }
    if (self->callbacksForThisFrameWereProcessedEvent) {
        il2cpp_utils::RunMethod(self->callbacksForThisFrameWereProcessedEvent, il2cpp_utils::FindMethodUnsafe("System", "Action", "Invoke", 0));
        // self->callbacksForThisFrameWereProcessedEvent->Invoke();
    }

}


void NoodleExtensions::InstallBeatmapObjectCallbackControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, LateUpdate, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectCallbackController", "LateUpdate", 0));
}