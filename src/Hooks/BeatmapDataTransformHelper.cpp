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

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

static Il2CppClass *customObstacleDataClass;
static Il2CppClass *customNoteDataClass;

float ObjectSortGetTime(BeatmapObjectData *n) {
    if (n->klass == customObstacleDataClass) {
        auto *obstacle = reinterpret_cast<CustomJSONData::CustomObstacleData *>(n);
        return n->time - getAD(obstacle->customData).aheadTime;
    } else if (n->klass == customNoteDataClass) {
        auto *note = reinterpret_cast<CustomJSONData::CustomNoteData *>(n);
        return n->time - getAD(note->customData).aheadTime;
    } else {
        return n->time;
    }
}

bool ObjectTimeCompare(BeatmapObjectData *a, BeatmapObjectData *b) {
    return ObjectSortGetTime(a) < ObjectSortGetTime(b);
}

void OrderObjects(List<BeatmapObjectData *> *beatmapObjectsData) {
    BeatmapObjectData **begin = beatmapObjectsData->items.begin();
    BeatmapObjectData **end = begin + beatmapObjectsData->get_Count();
    std::sort(begin, end, ObjectTimeCompare);
}

IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
    BeatmapData *customBeatmapData = beatmapData->GetCopy();
    customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    float const startHalfJumpDurationInBeats = 4;
    float const maxHalfJumpDistance = 18;
    float const moveDuration = 0.5f;

    // loop through all objects in all lines of the beatmapData
    for (BeatmapLineData *beatmapLineData : customBeatmapData->beatmapLinesData) {
        if (!beatmapLineData)
            continue;

        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData = beatmapLineData->beatmapObjectsData->items.get(j);
            float bpm;

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto *obstacleData = (CustomJSONData::CustomObstacleData *)beatmapObjectData;
                customDataWrapper = obstacleData->customData;
                bpm = obstacleData->bpm;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto *noteData = (CustomJSONData::CustomNoteData *)beatmapObjectData;
                customDataWrapper = noteData->customData;
                bpm = noteData->bpm;
            } else {
                continue;
            }

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
        }

        OrderObjects(beatmapLineData->beatmapObjectsData);
    }

    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
}

void LoadNoodleObjects(CustomJSONData::CustomBeatmapData* beatmap) {
    NELogger::GetLogger().info("BeatmapData klass name is %s",
                               beatmap->klass->name);

    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    auto &beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

    if (!beatmapAD.valid) {
        TracksAD::readBeatmapDataAD(beatmap);
    }

    for (BeatmapLineData *beatmapLineData : beatmap->beatmapLinesData) {
        if (!beatmapLineData)
            continue;

        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData =
                    beatmapLineData->beatmapObjectsData->items.get(j);

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto obstacleData =
                        (CustomJSONData::CustomObstacleData *)beatmapObjectData;
                customDataWrapper = obstacleData->customData;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData =
                        (CustomJSONData::CustomNoteData *)beatmapObjectData;
                customDataWrapper = noteData->customData;
            } else {
                continue;
            }

            if (customDataWrapper->value) {
                rapidjson::Value &customData = *customDataWrapper->value;
                BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);

                if (ad.parsed)
                    continue;

                ad.objectData = ObjectCustomData(customData, ad.flip);

                if (customData.HasMember("_animation")) {
                    rapidjson::Value &animation = customData["_animation"];
                    ad.animationData = AnimationObjectData(beatmapAD, animation);
                } else {
                    ad.animationData = AnimationObjectData();
                }
                ad.parsed = true;
            }
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
    rapidjson::Value &eventData = *customEventData->data;
    auto& eventAD = getEventAD(customEventData);

    if (eventAD.parsed)
        return;

    if (typeHash == jsonNameHash_AssignTrackParent) {
        std::string parentTrackName(eventData["_parentTrack"].GetString());
        Track* track = &beatmapAD.tracks[parentTrackName];

        rapidjson::Value &rawChildrenTracks = eventData["_childrenTracks"];
        std::vector<Track *> childrenTracks;
        childrenTracks.reserve(rawChildrenTracks.Size());
        for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin();
             itr != rawChildrenTracks.End(); itr++) {
            Track *child = &beatmapAD.tracks[itr->GetString()];
            // NELogger::GetLogger().debug("Assigning track %s(%p) to parent track %s(%p)", itr->GetString(), child, eventData["_parentTrack"].GetString(), track);
            childrenTracks.push_back(child);
        }

        // copy constructor deleted for some reason???
        eventAD.parentTrackEventData.emplace(eventData, childrenTracks, parentTrackName, track);

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
    for (auto const& customEventData : *beatmap->customEventsData) {
        LoadNoodleEvent(beatmapAD, &customEventData);
    }
}

MAKE_HOOK_MATCH(BeatmapDataTransformHelper_CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData, IReadonlyBeatmapData *,
                IReadonlyBeatmapData *beatmapData, IPreviewBeatmapLevel *beatmapLevel,
                GameplayModifiers *gameplayModifiers, PracticeSettings *practiceSettings,
                bool leftHanded, EnvironmentEffectsFilterPreset environmentEffectsFilterPreset,
                EnvironmentIntensityReductionOptions *environmentIntensityReductionOptions,
                bool screenDisplacementEffectsEnabled) {
    auto result = BeatmapDataTransformHelper_CreateTransformedBeatmapData(
        beatmapData, beatmapLevel, gameplayModifiers, practiceSettings, leftHanded,
        environmentEffectsFilterPreset, environmentIntensityReductionOptions,
        screenDisplacementEffectsEnabled);

    if (!Hooks::isNoodleHookEnabled())
        return result;

    LoadNoodleObjects(reinterpret_cast<CustomJSONData::CustomBeatmapData *>(result));
    auto *transformedBeatmapData = ReorderLineData(result);

    LoadNoodleEvents(reinterpret_cast<CustomJSONData::CustomBeatmapData *>(transformedBeatmapData));

    return transformedBeatmapData;
}

void InstallBeatmapDataTransformHelperHooks(Logger &logger) {
    INSTALL_HOOK_ORIG(logger, BeatmapDataTransformHelper_CreateTransformedBeatmapData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);