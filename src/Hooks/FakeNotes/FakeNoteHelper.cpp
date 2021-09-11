#include "FakeNoteHelper.h"

#include "GlobalNamespace/NoteController.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NELogger.h"
#include "AssociatedData.h"

using namespace GlobalNamespace;

bool FakeNoteHelper::GetFakeNote(NoteController *noteController) {
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(noteController->get_noteData());
    if (!customNoteData->customData->value) {
        return false;
    }
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    return ad.objectData.fake && *ad.objectData.fake;
}

bool FakeNoteHelper::GetCuttable(NoteData *noteData) {
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(noteData);
    if (!customNoteData->customData->value) {
        return true;
    }
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    return !ad.objectData.interactable || *ad.objectData.interactable;
}