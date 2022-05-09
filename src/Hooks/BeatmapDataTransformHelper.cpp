#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

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

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "NECaches.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "GlobalNamespace/SortedList_1.hpp"

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

static Il2CppClass *customObstacleDataClass;
static Il2CppClass *customNoteDataClass;

float ObjectSortGetTime(BeatmapObjectData const* n) {
    if (n->klass == customObstacleDataClass) {
        auto *obstacle = reinterpret_cast<CustomJSONData::CustomObstacleData const*>(n);
        return n->time - getAD(obstacle->customData).aheadTime;
    } else if (n->klass == customNoteDataClass) {
        auto *note = reinterpret_cast<CustomJSONData::CustomNoteData const*>(n);
        return n->time - getAD(note->customData).aheadTime;
    } else {
        return n->time;
    }
}

constexpr bool ObjectTimeCompare(BeatmapObjectData const * a, BeatmapObjectData const* b) {
    return ObjectSortGetTime(a) < ObjectSortGetTime(b);
}

void OrderObjects(List<BeatmapObjectData *> *beatmapObjectsData) {
    BeatmapObjectData **begin = beatmapObjectsData->items.begin();
    BeatmapObjectData **end = begin + beatmapObjectsData->get_Count();
    std::stable_sort(begin, end, ObjectTimeCompare);
}
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

extern System::Collections::Generic::LinkedList_1<BeatmapDataItem*>* SortAndOrderList(CustomJSONData::CustomBeatmapData* beatmapData);

void LoadNoodleObjects(CustomJSONData::CustomBeatmapData* beatmap) {
    NELogger::GetLogger().info("BeatmapData klass name is %s",
                               beatmap->klass->name);

    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    auto &beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(beatmap);
    }


    auto notes = beatmap->GetBeatmapItemsCpp<NoteData*>();
    auto obstacles = beatmap->GetBeatmapItemsCpp<ObstacleData*>();

    std::vector<BeatmapObjectData*> objects;
    objects.reserve(notes.size() + obstacles.size());

    std::copy(notes.begin(), notes.end(), std::back_inserter(objects));
    std::copy(obstacles.begin(), obstacles.end(), std::back_inserter(objects));

    for (BeatmapObjectData *beatmapObjectData : objects) {
        CustomJSONData::CustomNoteData *noteData;
        CustomJSONData::JSONWrapper *customDataWrapper;
        float bpm;
        if (beatmapObjectData->klass == customObstacleDataClass) {
            auto *obstacleData = (CustomJSONData::CustomObstacleData *) beatmapObjectData;
            customDataWrapper = obstacleData->customData;
            bpm = obstacleData->bpm;
        } else if (beatmapObjectData->klass == customNoteDataClass) {
            auto *noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
            customDataWrapper = noteData->customData;
            bpm = noteData->bpm;
        } else {
            continue;
        }

        float const startHalfJumpDurationInBeats = 4;
        float const maxHalfJumpDistance = 18;
        float const moveDuration = 0.5f;

        BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);
        float njs = ad.objectData.noteJumpMovementSpeed.value_or(NECaches::noteJumpMovementSpeed);
        float spawnOffset = ad.objectData.noteJumpStartBeatOffset.value_or(NECaches::noteJumpStartBeatOffset);

        float num = 60.0f / bpm;
        float num2 = startHalfJumpDurationInBeats;
        while (njs * num * num2 > maxHalfJumpDistance) {
            num2 /= 2.0f;
        }

        num2 += spawnOffset;
        if (num2 < 1.0f) {
            num2 = 1.0f;
        }

        float jumpDuration = num * num2 * 2;
        ad.aheadTime = moveDuration + (jumpDuration * 0.5f);

        if (customDataWrapper->value) {
            rapidjson::Value const &customData = *customDataWrapper->value;
            BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);

            if (ad.parsed)
                continue;

            ad.objectData = ObjectCustomData(customData, ad.flip, noteData);

            if (customData.HasMember("_animation")) {
                rapidjson::Value const &animation = customData["_animation"];
                ad.animationData = AnimationObjectData(beatmapAD, animation);
            } else {
                ad.animationData = AnimationObjectData();
            }
            ad.parsed = true;
        }
    }
}

void LoadNoodleEvent(TracksAD::BeatmapAssociatedData &beatmapAD, CustomJSONData::CustomEventData const* customEventData) {
    bool isType = false;

    auto typeHash = customEventData->typeHash;

#define TYPE_GET(jsonName, varName)                                \
    static auto jsonNameHash_##varName = std::hash<std::string_view>()(jsonName); \
    if (!isType && typeHash == (jsonNameHash_##varName))                      \
        isType = true;

    TYPE_GET("AssignTrackParent", AssignTrackParent)
    TYPE_GET("AssignPlayerToTrack", AssignPlayerToTrack)

    if (!isType) {
        return;
    }
    rapidjson::Value const& eventData = *customEventData->data;
    auto& eventAD = getEventAD(customEventData);

    if (eventAD.parsed)
        return;

    if (typeHash == jsonNameHash_AssignTrackParent) {
        eventAD.parentTrackEventData.emplace(eventData, beatmapAD);
    } else if (typeHash == jsonNameHash_AssignPlayerToTrack) {
        std::string trackName(eventData["_track"].GetString());
        Track *track = &beatmapAD.tracks[trackName];
        NELogger::GetLogger().debug("Assigning player to track %s at %p",
                                    trackName.c_str(), track);
        eventAD.playerTrackEventData.emplace(track);
    }

    eventAD.parsed = true;
}

void LoadNoodleEvents(CustomJSONData::CustomBeatmapData* beatmap) {
    auto &beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(beatmap);
    }

    // Parse events
    for (auto const& customEventData : beatmap->GetBeatmapItemsCpp<CustomJSONData::CustomEventData*>()) {
        LoadNoodleEvent(beatmapAD, customEventData);
    }
}

MAKE_HOOK_MATCH(BeatmapDataTransformHelper_CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData, IReadonlyBeatmapData *,
                GlobalNamespace::IReadonlyBeatmapData* beatmapData, ::GlobalNamespace::IPreviewBeatmapLevel* beatmapLevel,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers, bool leftHanded,
                ::GlobalNamespace::EnvironmentEffectsFilterPreset environmentEffectsFilterPreset,
                ::GlobalNamespace::EnvironmentIntensityReductionOptions* environmentIntensityReductionOptions,
                ::GlobalNamespace::MainSettingsModelSO* mainSettingsModel) {
    auto result = BeatmapDataTransformHelper_CreateTransformedBeatmapData(
        beatmapData, beatmapLevel, gameplayModifiers, leftHanded,
        environmentEffectsFilterPreset, environmentIntensityReductionOptions,
        mainSettingsModel);

    if (!Hooks::isNoodleHookEnabled())
        return result;

    auto customBeatmap = reinterpret_cast<CustomJSONData::CustomBeatmapData *>(result);

    LoadNoodleObjects(reinterpret_cast<CustomJSONData::CustomBeatmapData *>(result));
    il2cpp_utils::cast<GlobalNamespace::SortedList_1<BeatmapDataItem*>>(customBeatmap->allBeatmapData)->items = SortAndOrderList(customBeatmap);
    auto *transformedBeatmapData = result; // ReorderLineData(result);

    LoadNoodleEvents(reinterpret_cast<CustomJSONData::CustomBeatmapData *>(transformedBeatmapData));

    return transformedBeatmapData;
}

void InstallBeatmapDataTransformHelperHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, BeatmapDataTransformHelper_CreateTransformedBeatmapData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);