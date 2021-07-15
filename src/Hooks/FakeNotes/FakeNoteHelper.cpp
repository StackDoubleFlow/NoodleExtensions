#include "FakeNoteHelper.h"

#include "GlobalNamespace/NoteController.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NELogger.h"

using namespace GlobalNamespace;

bool FakeNoteHelper::GetFakeNote(NoteController *noteController) {
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(noteController->get_noteData());
    if (!customNoteData->customData->value) {
        return false;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;
    return customData.HasMember("_fake") && customData["_fake"].GetBool();
}

bool FakeNoteHelper::GetCuttable(NoteData *noteData) {
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(noteData);
    if (!customNoteData->customData->value) {
        return true;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;
    return !customData.HasMember("_interactable") || customData["_interactable"].GetBool();
}