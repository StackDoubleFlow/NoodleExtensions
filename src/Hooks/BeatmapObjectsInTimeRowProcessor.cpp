#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "GlobalNamespace/BeatmapObjectsInTimeRowProcessor.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace CustomJSONData;

MAKE_HOOK_MATCH(BeatmapObjectsInTimeRowProcessor_ProcessAllNotesInTimeRow,
                &BeatmapObjectsInTimeRowProcessor::ProcessAllNotesInTimeRow, void,
                BeatmapObjectsInTimeRowProcessor *self, List<NoteData *> *notes) {

    auto *customNotes = reinterpret_cast<List<CustomNoteData *> *>(notes);
    std::unordered_map<float, std::vector<CustomNoteData *>> notesInColumn;
    for (int i = 0; i < customNotes->get_Count(); i++) {
        CustomNoteData *noteData = customNotes->items->values[i];

        float lineIndex = noteData->lineIndex - 2.0f;
        float lineLayer = noteData->noteLineLayer;
        if (noteData->customData->value) {
            rapidjson::Value &customData = *noteData->customData->value;
            auto pos = customData.FindMember("_position");
            if (pos != customData.MemberEnd()) {
                int size = pos->value.Size();
                if (size >= 1) {
                    lineIndex = pos->value[0].GetFloat();
                }
                if (size >= 2) {
                    lineLayer = pos->value[1].GetFloat();
                }
            }
        }

        std::vector<CustomNoteData *> &list = notesInColumn[lineIndex];

        bool flag = false;
        for (int k = 0; k < list.size(); k++) {
            float listLineLayer = list[k]->noteLineLayer;
            if (noteData->customData->value) {
                rapidjson::Value &customData = *noteData->customData->value;
                auto listPos = customData.FindMember("_position");
                if (listPos != customData.MemberEnd()) {
                    if (listPos->value.Size() >= 2) {
                        listLineLayer = listPos->value[1].GetFloat();
                    }
                }
            }

            if (listLineLayer > lineLayer) {
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
            BeatmapObjectAssociatedData &ad = getAD(list[m]->customData);
            ad.startNoteLineLayer = (float) m;
        }
    }

    BeatmapObjectsInTimeRowProcessor_ProcessAllNotesInTimeRow(self, notes);
}

void InstallBeatmapObjectsInTimeRowProcessorHooks(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapObjectsInTimeRowProcessor_ProcessAllNotesInTimeRow);
}

NEInstallHooks(InstallBeatmapObjectsInTimeRowProcessorHooks);