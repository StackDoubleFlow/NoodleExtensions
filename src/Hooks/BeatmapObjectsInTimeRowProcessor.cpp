#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "GlobalNamespace/BeatmapObjectsInTimeRowProcessor.hpp"
#include "GlobalNamespace/BeatmapObjectsInTimeRowProcessor_TimeSliceContainer_1.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/NoteCutDirectionExtensions.hpp"
#include "GlobalNamespace/Vector2Extensions.hpp"
#include "GlobalNamespace/BeatmapObjectsInTimeRowProcessor_SliderTailData.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/VList.h"

#include "NEUtils.hpp"
#include "tracks/shared/Json.h"

using namespace GlobalNamespace;
using namespace CustomJSONData;
//
//void BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSliceTranspile(BeatmapObjectsInTimeRowProcessor* self,
//               GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::BeatmapDataItem*>* allObjectsTimeSlice, float nextTimeSliceTime) {
//
//
//    auto notesInColumnsReusableProcessingListOfLists = self->notesInColumnsReusableProcessingListOfLists;
//    for (auto const &l: notesInColumnsReusableProcessingListOfLists) {
//        l->Clear();
//    }
//
//    auto items = VList(allObjectsTimeSlice->items);
//
//
//    auto enumerable = NoodleExtensions::of_type<NoteData *>(items);
//    auto enumerable2 = NoodleExtensions::of_type<SliderData *>(items);
//    auto enumerable3 = NoodleExtensions::of_type<BeatmapObjectsInTimeRowProcessor::SliderTailData *>(items);
//    for (auto noteData: enumerable) {
//
//        // TRANSPILE HERE
//        // CLAMP
//        auto list = VList(self->notesInColumnsReusableProcessingListOfLists[std::clamp(noteData->lineIndex, 0, 3)]);
//        // TRANSPILE HERE
//
//        bool flag = false;
//
//        for (int j = 0; j < list.size(); j++) {
//            if (list[j]->noteLineLayer > noteData->noteLineLayer) {
//                list.insert_at(j, noteData);
//                flag = true;
//                break;
//            }
//        }
//        if (!flag) {
//            list.push_back(noteData);
//        }
//    }
//    for (auto const& notesInColumnsReusableProcessingListOfList : self->notesInColumnsReusableProcessingListOfLists) {
//        auto list2 = VList(notesInColumnsReusableProcessingListOfList);
//        for (int l = 0; l < list2.size(); l++) {
//            list2[l]->SetBeforeJumpNoteLineLayer((NoteLineLayer) l);
//        }
//    }
//    for (auto sliderData: enumerable2) {
//        for (auto noteData2: enumerable) {
//            if (BeatmapObjectsInTimeRowProcessor::SliderHeadPositionOverlapsWithNote(sliderData, noteData2)) {
//                sliderData->SetHasHeadNote(true);
//                sliderData->SetHeadBeforeJumpLineLayer(noteData2->beforeJumpNoteLineLayer);
//                if (sliderData->sliderType == SliderData::Type::Burst) {
//                    noteData2->ChangeToBurstSliderHead();
//                    if (noteData2->cutDirection == sliderData->tailCutDirection) {
//                        auto line = StaticBeatmapObjectSpawnMovementData::Get2DNoteOffset(noteData2->lineIndex,
//                                                                                          self->numberOfLines,
//                                                                                          noteData2->noteLineLayer) -
//                                    StaticBeatmapObjectSpawnMovementData::Get2DNoteOffset(sliderData->tailLineIndex,
//                                                                                          self->numberOfLines,
//                                                                                          sliderData->tailLineLayer);
//                        float num = Vector2Extensions::SignedAngleToLine(
//                                NoteCutDirectionExtensions::Direction(noteData2->cutDirection), line);
//                        if (std::abs(num) <= 40.0f) {
//                            noteData2->SetCutDirectionAngleOffset(num);
//                            sliderData->SetCutDirectionAngleOffset(num, num);
//                        }
//                    }
//                } else {
//                    noteData2->ChangeToSliderHead();
//                }
//            }
//        }
//    }
//    for (auto sliderTailData: enumerable3) {
//        auto slider = sliderTailData->slider;
//        for (auto noteData3: enumerable) {
//            if (BeatmapObjectsInTimeRowProcessor::SliderTailPositionOverlapsWithNote(slider, noteData3)) {
//                slider->SetHasTailNote(true);
//                slider->SetTailBeforeJumpLineLayer(noteData3->beforeJumpNoteLineLayer);
//                noteData3->ChangeToSliderTail();
//            }
//        }
//
//    }
//}

MAKE_HOOK_MATCH(BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice,
                &BeatmapObjectsInTimeRowProcessor::HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice,
                void,
                BeatmapObjectsInTimeRowProcessor* self,
                GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::BeatmapDataItem*>* allObjectsTimeSlice, float nextTimeSliceTime) {
    if (!Hooks::isNoodleHookEnabled())
        return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice(self,
                                                                                                           allObjectsTimeSlice,
                                                                                                           nextTimeSliceTime);

    auto items = allObjectsTimeSlice->items;

    std::vector<CustomNoteData*> customNotes = NoodleExtensions::of_type<CustomNoteData*>(VList(items));

    if (customNotes.empty())
        return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice(self, allObjectsTimeSlice, nextTimeSliceTime);




    std::unordered_map<float, std::vector<CustomNoteData *>> notesInColumn;
    for (auto noteData : customNotes) {
        float lineIndex = noteData->lineIndex - 2.0f;
        float lineLayer = noteData->noteLineLayer;
        if (noteData->customData->value) {
            rapidjson::Value const& customData = *noteData->customData->value;

            auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
            if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::POSITION.data());

            lineIndex = pos.first.value_or(lineIndex);
            lineLayer = pos.second.value_or(lineLayer);
        }

        std::vector<CustomNoteData *> &list = notesInColumn[lineIndex];

        bool flag = false;
        for (int k = 0; k < list.size(); k++) {
            float listLineLayer = list[k]->noteLineLayer;
            auto kNote = list[k];

            if (kNote->customData->value) {
                rapidjson::Value const& customData = *kNote->customData->value;

                auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
                if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::POSITION.data());

                listLineLayer = pos.second.value_or(listLineLayer = pos.second.value());
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

        BeatmapObjectAssociatedData &ad = getAD(noteData->customData);
        if (noteData->customData->value) {
            rapidjson::Value const &customData = *noteData->customData->value;

            auto pair = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_FLIP);

            if (!pair.first) {
                pair = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::FLIP);
            }

            auto [x, y] = pair;

            ad.flipX = x;
            ad.flipY = y;
        }

        if (!ad.flipY) {
            ad.flipX = lineIndex;
            ad.flipY = 0;
        }
    }

    for (auto &[_, list] : notesInColumn) {
        for (int m = 0; m < list.size(); m++) {
            BeatmapObjectAssociatedData &ad = getAD(list[m]->customData);
            ad.startNoteLineLayer = (float) m;
        }
    }

    return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice(self, allObjectsTimeSlice, nextTimeSliceTime);
}

MAKE_HOOK_MATCH(BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow,
                &BeatmapObjectsInTimeRowProcessor::HandleCurrentTimeSliceColorNotesDidFinishTimeSlice, void,
                BeatmapObjectsInTimeRowProcessor* self,
                GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::NoteData*>* currentTimeSlice,
                float nextTimeSliceTime) {
    if (!Hooks::isNoodleHookEnabled())
        return BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow(self, currentTimeSlice, nextTimeSliceTime);

    auto items = VList(reinterpret_cast<List<CustomNoteData *> *>(currentTimeSlice->items));

    auto colorNotesData = NoodleExtensions::of_type<CustomNoteData*>(items);

    int const customNoteCount = colorNotesData.size();

    if (customNoteCount == 2) {
        std::array<float, 2> lineIndexes{}, lineLayers{};

        for (int i = 0; i < customNoteCount; i++) {
            CustomNoteData* noteData = colorNotesData[i];

            float lineIndex = noteData->lineIndex - 2.0f;
            float lineLayer = noteData->noteLineLayer;
            if (noteData->customData->value) {
                rapidjson::Value const& customData = *noteData->customData->value;
                auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
                if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::POSITION.data());
                lineIndex = pos.first.value_or(lineIndex);
                lineLayer = pos.second.value_or(lineLayer);
            }

            lineIndexes[i] = lineIndex;
            lineLayers[i] = lineLayer;
        }

        auto firstNote = colorNotesData[0];
        auto secondNote = colorNotesData[1];

        if (firstNote->colorType != secondNote->colorType &&
            ((firstNote->colorType == ColorType::ColorA && lineIndexes[0] > lineIndexes[1]) ||
             (firstNote->colorType == ColorType::ColorB && lineIndexes[0] < lineIndexes[1]))) {
            for (int i = 0; i < customNoteCount; i++) {
                // god aero I hate the mess of code you've made

                auto noteData = colorNotesData[i];

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
                noteAD.flipX = flipVec.x;
                noteAD.flipY = flipVec.y;
            }
        }
    }


    BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow(self, currentTimeSlice, nextTimeSliceTime);
}


void InstallBeatmapObjectsInTimeRowProcessorHooks(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapObjectsInTimeRowProcessor_ProcessColorNotesInTimeRow);

    INSTALL_HOOK(logger, BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice)
}

NEInstallHooks(InstallBeatmapObjectsInTimeRowProcessorHooks);