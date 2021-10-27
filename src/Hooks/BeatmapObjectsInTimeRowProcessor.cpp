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

    if (notes->get_Count() < 0 || !il2cpp_utils::AssignableFrom<CustomNoteData*>(notes->get_Item(0)->klass))
        return BeatmapObjectsInTimeRowProcessor_ProcessAllNotesInTimeRow(self, notes);

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

MAKE_HOOK_MATCH(BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow,
                &BeatmapObjectsInTimeRowProcessor::ProcessColorNotesInTimeRow, void,
                System::Collections::Generic::IList_1<NoteData *> *colorNotesDataOld,
                float nextBasicNoteTimeRowTime) {
    List<CustomNoteData *> *colorNotesData = reinterpret_cast<List<CustomNoteData *> *>(colorNotesDataOld);

    int const customNoteCount = colorNotesData->get_Count();

    if (customNoteCount == 2) {
        std::array<float, 2> lineIndexes{}, lineLayers{};

        for (int i = 0; i < customNoteCount; i++) {
            auto noteData = colorNotesData->get_Item(i);

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

            lineIndexes[i] = lineIndex;
            lineLayers[i] = lineLayer;
        }

        auto firstNote = colorNotesData->get_Item(0);
        auto secondNote = colorNotesData->get_Item(1);

        if (firstNote->get_colorType() != secondNote->get_colorType() &&
            ((firstNote->get_colorType() == ColorType::ColorA && lineIndexes[0] > lineIndexes[1]) ||
             (firstNote->get_colorType() == ColorType::ColorB && lineIndexes[0] < lineIndexes[1]))) {
            for (int i = 0; i < customNoteCount; i++) {
                // god aero I hate the mess of code you've made

                auto noteData = colorNotesData->get_Item(i);

                NEVector::Vector2 flipVec;

                flipVec.x = lineIndexes[1 - i];


                float flipYSide = (lineIndexes[i] > lineIndexes[1 - i]) ? 1 : -1;
                if ((lineIndexes[i] > lineIndexes[1 - i] && lineLayers[i] < lineLayers[1 - i]) ||
                    (lineIndexes[i] < lineIndexes[1 - i] &&
                     lineLayers[i] > lineLayers[1 - i])) {
                    flipYSide *= -1.0f;
                }

                flipVec.y = flipYSide;

                auto &noteAD = getAD(noteData->customData);
                noteAD.objectData.flip = flipVec;
            }
        }
    }


    BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow(colorNotesDataOld, nextBasicNoteTimeRowTime);
}


void InstallBeatmapObjectsInTimeRowProcessorHooks(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapObjectsInTimeRowProcessor_ProcessAllNotesInTimeRow);
    INSTALL_HOOK(logger, BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow);
}

NEInstallHooks(InstallBeatmapObjectsInTimeRowProcessorHooks);