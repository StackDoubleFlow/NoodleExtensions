#include "FakeNoteHelper.h"

#include "GlobalNamespace/NoteController.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NELogger.h"

using namespace GlobalNamespace;

bool FakeNoteHelper::GetFakeNote(NoteController *noteController) {
    // NELogger::GetLogger().info("In GetFakeNote");
    // NELogger::GetLogger().info("noteData pointer %p", noteController->noteData);
    // NELogger::GetLogger().info("GetFakeNote noteData klass %s", noteController->noteData->klass->name);
    auto customNoteData = (CustomJSONData::CustomNoteData *) noteController->noteData;
    if (!customNoteData->customData->value) {
        return false;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;
    return customData.HasMember("_fake") && customData["_fake"].GetBool();
}

bool FakeNoteHelper::GetCuttable(NoteData *noteData) {
    auto customNoteData = (CustomJSONData::CustomNoteData *) noteData;
    if (!customNoteData->customData->value) {
        return true;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;
    return !customData.HasMember("_interactable") || customData["_interactable"].GetBool();
}