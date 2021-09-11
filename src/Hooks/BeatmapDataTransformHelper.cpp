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
#include "custom-json-data/shared/CustomBeatmapData.h"

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

// GameplayCoreInstaller.cpp
extern int CachedNoteJumpMovementSpeed;
extern int CachedNoteJumpStartBeatOffset;

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
    BeatmapObjectData **begin = beatmapObjectsData->items->values;
    BeatmapObjectData **end = begin + beatmapObjectsData->get_Count();
    std::sort(begin, end, ObjectTimeCompare);
}

IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
    BeatmapData *customBeatmapData = beatmapData->GetCopy();
    customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    customNoteDataClass = classof(CustomJSONData::CustomNoteData *);

    float startHalfJumpDurationInBeats = 4;
    float maxHalfJumpDistance = 18;
    float moveDuration = 0.5f;

    // loop through all objects in all lines of the beatmapData
    for (int i = 0; i < customBeatmapData->beatmapLinesData->Length(); i++) {
        BeatmapLineData *beatmapLineData = customBeatmapData->beatmapLinesData->values[i];
        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData =
                beatmapLineData->beatmapObjectsData->items->values[j];
            float bpm;

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto obstacleData = (CustomJSONData::CustomObstacleData *)beatmapObjectData;
                customDataWrapper = obstacleData->customData;
                bpm = obstacleData->bpm;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData = (CustomJSONData::CustomNoteData *)beatmapObjectData;
                customDataWrapper = noteData->customData;
                bpm = noteData->bpm;
            } else {
                continue;
            }

            BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);
            float &aheadTime = ad.aheadTime;

            float njs;
            float spawnOffset;
            if (customDataWrapper->value) {
                rapidjson::Value &customData = *customDataWrapper->value;
                njs = ad.objectData.noteJumpMovementSpeed.value_or(CachedNoteJumpMovementSpeed);
                spawnOffset = ad.objectData.noteJumpStartBeatOffset.value_or(CachedNoteJumpStartBeatOffset);
            } else {
                njs = CachedNoteJumpMovementSpeed;
                spawnOffset = CachedNoteJumpStartBeatOffset;
            }

            float num = 60 / bpm;
            float num2 = startHalfJumpDurationInBeats;
            while (njs * num * num2 > maxHalfJumpDistance) {
                num2 /= 2;
            }

            num2 += spawnOffset;
            if (num2 < 1) {
                num2 = 1;
            }

            float jumpDuration = num * num2 * 2;
            aheadTime = moveDuration + (jumpDuration * 0.5f);
        }

        OrderObjects(beatmapLineData->beatmapObjectsData);
    }

    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
}

MAKE_HOOK_MATCH(BeatmapDataTransformHelper_CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData, IReadonlyBeatmapData *,
                IReadonlyBeatmapData *beatmapData, IPreviewBeatmapLevel *beatmapLevel,
                GameplayModifiers *gameplayModifiers, PracticeSettings *practiceSettings,
                bool leftHanded, EnvironmentEffectsFilterPreset environmentEffectsFilterPreset,
                EnvironmentIntensityReductionOptions *environmentIntensityReductionOptions,
                bool screenDisplacementEffectsEnabled) {
    auto *transformedBeatmapData = ReorderLineData(beatmapData);
    return BeatmapDataTransformHelper_CreateTransformedBeatmapData(
        transformedBeatmapData, beatmapLevel, gameplayModifiers, practiceSettings, leftHanded,
        environmentEffectsFilterPreset, environmentIntensityReductionOptions,
        screenDisplacementEffectsEnabled);
}

// Skip obstacle merging, I have no clue how much this can fuck things
MAKE_HOOK_MATCH(BeatmapDataObstaclesMergingTransform_CreateTransformedData,
                &BeatmapDataObstaclesMergingTransform::CreateTransformedData,
                IReadonlyBeatmapData *, IReadonlyBeatmapData *beatmapData) {
    return beatmapData;
}

void InstallBeatmapDataTransformHelperHooks(Logger &logger) {
    INSTALL_HOOK(logger, BeatmapDataTransformHelper_CreateTransformedBeatmapData);
    INSTALL_HOOK(logger, BeatmapDataObstaclesMergingTransform_CreateTransformedData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);