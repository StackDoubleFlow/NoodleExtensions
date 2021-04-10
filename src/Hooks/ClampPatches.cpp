#include "GlobalNamespace/BeatmapData_-get_beatmapObjectsData-d__31.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "System/Int32.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <limits>
#include <map>

using namespace GlobalNamespace;

int addBeatmapObjectDataLineIndex;
MAKE_HOOK_OFFSETLESS(BeatmapData_AddBeatmapObjectData, void, BeatmapData* self, BeatmapObjectData* item)
{
    addBeatmapObjectDataLineIndex = item->lineIndex;
    // Preprocess the lineIndex to be 0-3 (the real method is hard-coded to 4 lines), recording the info needed to reverse it
    if (addBeatmapObjectDataLineIndex > 3) {
        item->lineIndex = 3;
    } else if (addBeatmapObjectDataLineIndex < 0) {
        item->lineIndex = 0;
    }

    BeatmapData_AddBeatmapObjectData(self, item);
}
MAKE_HOOK_OFFSETLESS(BeatmapLineData_AddBeatmapObjectData, void, BeatmapLineData* self, BeatmapObjectData* item)
{
    item->lineIndex = addBeatmapObjectDataLineIndex;
    BeatmapLineData_AddBeatmapObjectData(self, item);
}

MAKE_HOOK_OFFSETLESS(NoteProcessorClampPatch, void,
    NotesInTimeRowProcessor* self, List<NoteData*>* notes)
{
    std::map<int, int> extendedLanesMap;
    for (int i = 0; i < notes->size; ++i) {
        auto* item = notes->items->values[i];
        if (item->lineIndex > 3) {
            extendedLanesMap[i] = item->lineIndex;
            item->lineIndex = 3;
        } else if (item->lineIndex < 0) {
            extendedLanesMap[i] = item->lineIndex;
            item->lineIndex = 0;
        }
    }

    NoteProcessorClampPatch(self, notes);

    for (int i = 0; i < notes->size; ++i) {
        if (extendedLanesMap.find(i) != extendedLanesMap.end()) {
            auto* item = notes->items->values[i];
            item->lineIndex = extendedLanesMap[i];
        }
    }
}

MAKE_HOOK_OFFSETLESS(BeatmapObjectsDataClampPatch, bool, BeatmapData::$get_beatmapObjectsData$d__31 *self) {
    int num = self->$$1__state;
    BeatmapData *beatmapData = self->$$4__this;
    if (num != 0) {
        if (num != 1) {
            return false;
        }
        self->$$1__state = -1;
        // Increment index in idxs with clamped lineIndex
        int lineIndex = self->$minBeatmapObjectData$5__4->lineIndex;
        int clampedLineIndex = lineIndex > 3 ? 3 : lineIndex < 0 ? 0 : lineIndex;
        self->$idxs$5__3->values[clampedLineIndex]++;
        self->$minBeatmapObjectData$5__4 = nullptr;
    } else {
        self->$$1__state = -1;
        self->$beatmapLinesData$5__2 = (Array<BeatmapLineData *> *) beatmapData->get_beatmapLinesData();
        self->$idxs$5__3 = Array<int>::NewLength(self->$beatmapLinesData$5__2->Length());
    }
    self->$minBeatmapObjectData$5__4 = nullptr;
    float num2 = std::numeric_limits<float>::max();
    for (int i = 0; i < self->$beatmapLinesData$5__2->Length(); i++) {
        int idx = self->$idxs$5__3->values[i];
        BeatmapLineData *lineData = self->$beatmapLinesData$5__2->values[i];
        if (idx < lineData->beatmapObjectsData->get_Count()) {
            BeatmapObjectData *beatmapObjectData = lineData->beatmapObjectsData->get_Item(idx);
            float time = beatmapObjectData->time;
            if (time < num2) {
                num2 = time;
                self->$minBeatmapObjectData$5__4 = beatmapObjectData;
            }
        }
    }
    if (self->$minBeatmapObjectData$5__4 == nullptr) {
        return false;
    }
    self->$$2__current = self->$minBeatmapObjectData$5__4;
    self->$$1__state = 1;
    return true;
}

void NoodleExtensions::InstallClampPatches(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, BeatmapObjectsDataClampPatch, 
        il2cpp_utils::FindMethodUnsafe("", "BeatmapData/<get_beatmapObjectsData>d__31", "MoveNext", 0));
    INSTALL_HOOK_OFFSETLESS(logger, NoteProcessorClampPatch, 
        il2cpp_utils::FindMethodUnsafe("", "NotesInTimeRowProcessor", "ProcessAllNotesInTimeRow", 1));
    INSTALL_HOOK_OFFSETLESS(logger, BeatmapData_AddBeatmapObjectData, 
        il2cpp_utils::FindMethodUnsafe("", "BeatmapData", "AddBeatmapObjectData", 1));
    INSTALL_HOOK_OFFSETLESS(logger, BeatmapLineData_AddBeatmapObjectData, 
        il2cpp_utils::FindMethodUnsafe("", "BeatmapLineData", "AddBeatmapObjectData", 1));
}