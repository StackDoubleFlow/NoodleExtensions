#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapData_-get_beatmapObjectsData-d__31.hpp"
#include "GlobalNamespace/NotesInTimeRowProcessor.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "System/Int32.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <limits>
#include <map>

using namespace GlobalNamespace;

int addBeatmapObjectDataLineIndex;
MAKE_HOOK_MATCH(BeatmapData_AddBeatmapObjectData,
                &BeatmapData::AddBeatmapObjectData, void, BeatmapData *self,
                BeatmapObjectData *item) {
    addBeatmapObjectDataLineIndex = item->lineIndex;
    // Preprocess the lineIndex to be 0-3 (the real method is hard-coded to 4
    // lines), recording the info needed to reverse it
    if (addBeatmapObjectDataLineIndex > 3) {
        item->lineIndex = 3;
    } else if (addBeatmapObjectDataLineIndex < 0) {
        item->lineIndex = 0;
    }

    BeatmapData_AddBeatmapObjectData(self, item);
}
MAKE_HOOK_MATCH(BeatmapLineData_AddBeatmapObjectData,
                &BeatmapLineData::AddBeatmapObjectData, void,
                BeatmapLineData *self, BeatmapObjectData *item) {
    item->lineIndex = addBeatmapObjectDataLineIndex;
    BeatmapLineData_AddBeatmapObjectData(self, item);
}

MAKE_HOOK_MATCH(NoteProcessorClampPatch,
                &NotesInTimeRowProcessor::ProcessAllNotesInTimeRow, void,
                NotesInTimeRowProcessor *self, List<NoteData *> *notes) {
    std::map<int, int> extendedLanesMap;
    for (int i = 0; i < notes->size; ++i) {
        auto *item = notes->items->values[i];
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
            auto *item = notes->items->values[i];
            item->lineIndex = extendedLanesMap[i];
        }
    }

    if (std::find_if(notes->items->values,
                     notes->items->values + notes->get_Count(),
                     [](NoteData *x) {
                         return x->lineIndex > 3 || x->lineIndex < 0;
                     }) == notes->items->values + notes->get_Count()) {
        return;
    }
    std::unordered_map<int, std::vector<NoteData *>> notesInColumn;

    for (int j = 0; j < notes->get_Count(); j++) {
        NoteData *noteData = notes->items->values[j];
        std::vector<NoteData *> &list = notesInColumn[noteData->lineIndex];
        bool flag = false;
        for (int k = 0; k < list.size(); k++) {
            if (list[k]->noteLineLayer > noteData->noteLineLayer) {
                list.insert(list.begin() + k, noteData);
                flag = true;
                break;
            }
        }
        if (!flag) {
            list.push_back(noteData);
        }
    }
    for (auto &pair : notesInColumn) {
        auto &list = pair.second;
        for (int m = 0; m < list.size(); m++) {
            list[m]->SetNoteStartLineLayer(m);
        }
    }
}

MAKE_HOOK_MATCH(BeatmapObjectsDataClampPatch,
                &BeatmapData::$get_beatmapObjectsData$d__31::MoveNext, bool,
                BeatmapData::$get_beatmapObjectsData$d__31 *self) {
    int num = self->$$1__state;
    BeatmapData *beatmapData = self->$$4__this;
    if (num != 0) {
        if (num != 1) {
            return false;
        }
        self->$$1__state = -1;
        // Increment index in idxs with clamped lineIndex
        int lineIndex = self->$minBeatmapObjectData$5__4->lineIndex;
        int clampedLineIndex = lineIndex > 3   ? 3
                               : lineIndex < 0 ? 0
                                               : lineIndex;
        self->$idxs$5__3->values[clampedLineIndex]++;
        self->$minBeatmapObjectData$5__4 = nullptr;
    } else {
        self->$$1__state = -1;
        self->$beatmapLinesData$5__2 =
            (Array<BeatmapLineData *> *)beatmapData->get_beatmapLinesData();
        self->$idxs$5__3 =
            Array<int>::NewLength(self->$beatmapLinesData$5__2->Length());
    }
    self->$minBeatmapObjectData$5__4 = nullptr;
    float num2 = std::numeric_limits<float>::max();
    for (int i = 0; i < self->$beatmapLinesData$5__2->Length(); i++) {
        int idx = self->$idxs$5__3->values[i];
        BeatmapLineData *lineData = self->$beatmapLinesData$5__2->values[i];
        if (idx < lineData->beatmapObjectsData->get_Count()) {
            BeatmapObjectData *beatmapObjectData =
                lineData->beatmapObjectsData->get_Item(idx);
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

void InstallClampPatches(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapObjectsDataClampPatch);
    INSTALL_HOOK(logger, NoteProcessorClampPatch);
    INSTALL_HOOK(logger, BeatmapData_AddBeatmapObjectData);
    INSTALL_HOOK(logger, BeatmapLineData_AddBeatmapObjectData);
}

NEInstallHooks(InstallClampPatches);