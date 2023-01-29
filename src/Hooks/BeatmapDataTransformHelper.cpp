#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/IJumpOffsetYProvider.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"
#include "GlobalNamespace/PlayerHeightToJumpOffsetYProvider.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataObstaclesMergingTransform.hpp"
#include "GlobalNamespace/BeatmapDataTransformHelper.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/EnvironmentEffectsFilterPreset.hpp"
#include "GlobalNamespace/EnvironmentIntensityReductionOptions.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "System/Collections/Generic/IEnumerable_1.hpp"
#include "System/Func_2.hpp"
#include "System/Linq/Enumerable.hpp"
#include "System/Linq/IOrderedEnumerable_1.hpp"
#include "UnityEngine/Resources.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"
#include "GlobalNamespace/SortedList_1.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "SpawnDataHelper.h"
#include "tracks/shared/Json.h"

#include "songloader/shared/API.hpp"

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

//
//IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
//    auto *customBeatmapData = static_cast<CustomJSONData::CustomBeatmapData *>(beatmapData->GetCopy());
//    if (!customObstacleDataClass) {
//        customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
//        customNoteDataClass = classof(CustomJSONData::CustomNoteData *);
//    }
//
//
//
//    auto notes = customBeatmapData->GetBeatmapItemsCpp<NoteData*>();
//    auto obstacles = customBeatmapData->GetBeatmapItemsCpp<ObstacleData*>();
//
//    std::vector<BeatmapObjectData*> objects;
//    objects.reserve(notes.size() + obstacles.size());
//
//    std::copy(notes.begin(), notes.end(), std::back_inserter(objects));
//    std::copy(obstacles.begin(), obstacles.end(), std::back_inserter(objects));
//
//    // loop through all objects in all lines of the beatmapData
//    for (BeatmapObjectData *beatmapObjectData : objects) {
//
//    }
//
//    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
//}

extern System::Collections::Generic::LinkedList_1<BeatmapDataItem *> *
SortAndOrderList(CustomJSONData::CustomBeatmapData *beatmapData);


void LoadNoodleObjects(CustomJSONData::CustomBeatmapData *beatmap) {
    NELogger::GetLogger().info("BeatmapData klass name is %s",
                               beatmap->klass->name);

    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);
    static auto *customSliderDataClass = classof(CustomJSONData::CustomSliderData *);

    TracksAD::BeatmapAssociatedData &beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(beatmap);
    }

    auto v2 = beatmap->v2orEarlier;

    bool mirror = true;

    if (beatmap->customData->value) {
        rapidjson::Value const &data = *beatmap->customData->value;
        mirror = NEJSON::ReadOptionalBool(data, v2 ? "_questNoteMirror" : "questNoteMirror").value_or(true);
    }


    for (BeatmapObjectData *beatmapObjectData: beatmap->beatmapObjectDatas) {
        CustomJSONData::CustomNoteData *noteData = nullptr;
        CustomJSONData::CustomObstacleData *obstacleData = nullptr;
        CustomJSONData::CustomSliderData *sliderData = nullptr;
        CustomJSONData::JSONWrapper *customDataWrapper = nullptr;
        if (beatmapObjectData->klass == customObstacleDataClass) {
            obstacleData = (CustomJSONData::CustomObstacleData *) beatmapObjectData;
            customDataWrapper = obstacleData->customData;
        } else if (beatmapObjectData->klass == customNoteDataClass) {
            noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
            customDataWrapper = noteData->customData;
        } else if (beatmapObjectData->klass == customSliderDataClass) {
            sliderData = (CustomJSONData::CustomSliderData *) beatmapObjectData;
            customDataWrapper = sliderData->customData;
            continue;
        }


        BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);

        ad.mirror = mirror;

        if (customDataWrapper->value) {
            rapidjson::Value const &customData = *customDataWrapper->value;

            if (ad.parsed)
                continue;

            auto v3Fake = ad.objectData.fake;
            ad.objectData = ObjectCustomData(customData, noteData, obstacleData, v2);
            if (v3Fake) ad.objectData.fake = v3Fake;

            if (!ad.flipX || !ad.flipY) {
                auto [x, y] = NEJSON::ReadOptionalPair(customData, v2 ? NoodleExtensions::Constants::V2_FLIP
                                                                      : NoodleExtensions::Constants::FLIP);
                ad.flipX = x;
                ad.flipY = y;
            }

            auto animationKey = v2 ? NoodleExtensions::Constants::V2_ANIMATION
                                   : NoodleExtensions::Constants::ANIMATION;
            if (customData.HasMember(animationKey.data())) {
                rapidjson::Value const &animation = customData[animationKey.data()];
                ad.animationData = {beatmapAD, animation, v2};
            } else {
                ad.animationData = AnimationObjectData();
            }
        }
        ad.parsed = true;
    }

}

void LoadNoodleEvent(TracksAD::BeatmapAssociatedData &beatmapAD, CustomJSONData::CustomEventData const *customEventData,
                     bool v2) {
    bool isType = false;

    auto typeHash = customEventData->typeHash;

#define TYPE_GET(jsonName, varName)                                \
    static auto jsonNameHash_##varName = std::hash<std::string_view>()(jsonName); \
    if (!isType && typeHash == (jsonNameHash_##varName))                      \
        isType = true;

    TYPE_GET(NoodleExtensions::Constants::ASSIGN_TRACK_PARENT, AssignTrackParent)
    TYPE_GET(NoodleExtensions::Constants::ASSIGN_PLAYER_TO_TRACK, AssignPlayerToTrack)

    if (!isType) {
        return;
    }
    CRASH_UNLESS(customEventData->data);
    rapidjson::Value const &eventData = *customEventData->data;
    auto &eventAD = getEventAD(customEventData);

    if (eventAD.parsed)
        return;

    if (typeHash == jsonNameHash_AssignTrackParent) {
        eventAD.parentTrackEventData.emplace(eventData, beatmapAD, v2);
    } else if (typeHash == jsonNameHash_AssignPlayerToTrack) {
        std::string_view trackName(eventData[v2 ? NoodleExtensions::Constants::V2_TRACK.data()
                                                : NoodleExtensions::Constants::TRACK.data()].GetString());
        Track *track = &beatmapAD.tracks.try_emplace(std::string(trackName), v2).first->second;
        NELogger::GetLogger().debug("Assigning player to track %s at %p",
                                    trackName.data(), track);
        eventAD.playerTrackEventData.emplace(track);
    }

    eventAD.parsed = true;
}

void LoadNoodleEvents(CustomJSONData::CustomBeatmapData *beatmap) {
    auto &beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

    auto v2 = beatmap->v2orEarlier;

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(beatmap);
    }

    // Parse events
    for (auto const &customEventData: beatmap->customEventDatas) {
        LoadNoodleEvent(beatmapAD, customEventData, v2);
    }
}

MAKE_HOOK_MATCH(BeatmapDataTransformHelper_CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData, IReadonlyBeatmapData *,
                GlobalNamespace::IReadonlyBeatmapData *beatmapData,
                ::GlobalNamespace::IPreviewBeatmapLevel *beatmapLevel,
                ::GlobalNamespace::GameplayModifiers *gameplayModifiers, bool leftHanded,
                ::GlobalNamespace::EnvironmentEffectsFilterPreset environmentEffectsFilterPreset,
                ::GlobalNamespace::EnvironmentIntensityReductionOptions *environmentIntensityReductionOptions,
                ::GlobalNamespace::MainSettingsModelSO *mainSettingsModel) {
    auto result = BeatmapDataTransformHelper_CreateTransformedBeatmapData(
            beatmapData, beatmapLevel, gameplayModifiers, leftHanded,
            environmentEffectsFilterPreset, environmentIntensityReductionOptions,
            mainSettingsModel);

    if (!Hooks::isNoodleHookEnabled())
        return result;

    if (auto customBeatmap = il2cpp_utils::try_cast<CustomJSONData::CustomBeatmapData>(result)) {
        auto &beatmapAD = TracksAD::getBeatmapAD(customBeatmap.value()->customData);

        if (!beatmapAD.valid) {
            TracksAD::readBeatmapDataAD(*customBeatmap);
        }


        LoadNoodleObjects(*customBeatmap);
        LoadNoodleEvents(*customBeatmap);
    }


    return result;
}

void HandleFakeObjects(CustomJSONData::CustomLevelInfoSaveData *, std::string const &,
                       BeatmapSaveDataVersion3::BeatmapSaveData *unsafeBeatmap,
                       GlobalNamespace::BeatmapDataBasicInfo *basic) {
    using namespace CustomJSONData::v3;
    auto beatmap = il2cpp_utils::cast<CustomBeatmapSaveData>(unsafeBeatmap);
    if (!beatmap->levelCustomData || !beatmap->customData) return;

    auto noodle = NoodleExtensions::SceneTransitionHelper::CheckIfNoodle(*beatmap->levelCustomData);

    if (!noodle) return;

    rapidjson::Value const &customData = *beatmap->customData;

#define PARSE_ARRAY(key, array, parse) \
    auto key##it = customData.FindMember(#key); \
    if (key##it != customData.MemberEnd()) { \
        for (auto const& it : key##it->value.GetArray()) { \
            auto item = parse(it); \
            auto& ad = getAD(item->customData); \
            ad.objectData.fake = true; \
            array->Add(item); \
        } \
    }

    basic->obstaclesCount = unsafeBeatmap->obstacles->size;
    basic->cuttableNotesCount = unsafeBeatmap->colorNotes->size;
    basic->bombsCount = unsafeBeatmap->bombNotes->size;

    PARSE_ARRAY(fakeColorNotes, unsafeBeatmap->colorNotes, Parser::DeserializeColorNote);
    PARSE_ARRAY(fakeBombNotes, unsafeBeatmap->bombNotes, Parser::DeserializeBombNote);
    PARSE_ARRAY(fakeObstacles, unsafeBeatmap->obstacles, Parser::DeserializeObstacle);
    PARSE_ARRAY(fakeBurstSliders, unsafeBeatmap->burstSliders, Parser::DeserializeBurstSlider);
    PARSE_ARRAY(fakeSliders, unsafeBeatmap->sliders, Parser::DeserializeSlider);
}

void InstallBeatmapDataTransformHelperHooks(Logger &logger) {
    Modloader::requireMod("CustomJSONData");
    RuntimeSongLoader::API::AddBeatmapDataBasicInfoLoadedEvent(HandleFakeObjects);

    INSTALL_HOOK(logger, BeatmapDataTransformHelper_CreateTransformedBeatmapData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);