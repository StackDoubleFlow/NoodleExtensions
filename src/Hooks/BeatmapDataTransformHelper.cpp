#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
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

// BeatmapDataLoader.cpp
extern Il2CppClass *customObstacleDataClass;
extern Il2CppClass *customNoteDataClass;

System::Func_2<BeatmapObjectData *, float> *CreateOrderFunc() {
    std::vector<const Il2CppClass *> argClasses{classof(BeatmapObjectData *),
                                                classof(float)};
    auto genericClass = il2cpp_utils::MakeGeneric(
        il2cpp_utils::GetClassFromName("System", "Func`2"), argClasses);
    auto lambda = +[](BeatmapObjectData *n) {
        if (n->klass == customObstacleDataClass) {
            return n->time -
                   getAD(((CustomJSONData::CustomObstacleData *)n)->customData)
                       .aheadTime;
        } else if (n->klass == customNoteDataClass) {
            return n->time -
                   getAD(((CustomJSONData::CustomNoteData *)n)->customData)
                       .aheadTime;
        } else {
            return n->time;
        }
    };
    return il2cpp_utils::MakeDelegate<
        System::Func_2<BeatmapObjectData *, float> *>(
        genericClass, static_cast<Il2CppObject *>(nullptr), lambda);
}

List<BeatmapObjectData *> *
OrderObjects(List<BeatmapObjectData *> *beatmapObjectsData) {
    auto orderFunc = CreateOrderFunc();

    using IOrderedEnumerableT =
        System::Linq::IOrderedEnumerable_1<BeatmapObjectData *> *;
    // Used as a parameter for OrderBy
    auto enumerable = (IEnumerable_1<BeatmapObjectData *> *)beatmapObjectsData;

    // var orderedEnumerable beatmapObjectsData.OrderBy(orderFunc);
    auto orderByMethodInfo = il2cpp_utils::FindMethodUnsafe(
        "System.Linq", "Enumerable", "OrderBy", 2);
    auto orderByGenericMethodInfo = il2cpp_utils::MakeGenericMethod(
        orderByMethodInfo, {classof(BeatmapObjectData *), classof(float)});
    auto orderedEnumerable = CRASH_UNLESS(
        il2cpp_utils::RunMethod<IEnumerable_1<BeatmapObjectData *> *>(
            nullptr, orderByGenericMethodInfo, enumerable, orderFunc));
    // return orderedEnumerable.ToList();
    auto toListMethodInfo = il2cpp_utils::FindMethodUnsafe(
        "System.Linq", "Enumerable", "ToList", 1);
    auto toListGenericMethodInfo = il2cpp_utils::MakeGenericMethod(
        toListMethodInfo, {classof(BeatmapObjectData *)});
    return CRASH_UNLESS(il2cpp_utils::RunMethod<List<BeatmapObjectData *> *>(
        nullptr, toListGenericMethodInfo, orderedEnumerable));
}

IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
    BeatmapData *customBeatmapData = beatmapData->GetCopy();

    float startHalfJumpDurationInBeats = 4;
    float maxHalfJumpDistance = 18;
    float moveDuration = 0.5f;

    // loop through all objects in all lines of the beatmapData
    for (int i = 0; i < customBeatmapData->beatmapLinesData->Length(); i++) {
        BeatmapLineData *beatmapLineData =
            customBeatmapData->beatmapLinesData->values[i];
        for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
            BeatmapObjectData *beatmapObjectData =
                beatmapLineData->beatmapObjectsData->items->values[j];
            float bpm;
            float *aheadTime;

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                auto obstacleData =
                    (CustomJSONData::CustomObstacleData *)beatmapObjectData;
                customDataWrapper = obstacleData->customData;
                bpm = obstacleData->bpm;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData =
                    (CustomJSONData::CustomNoteData *)beatmapObjectData;
                customDataWrapper = noteData->customData;
                bpm = noteData->bpm;
            } else {
                continue;
            }

            aheadTime = &getAD(customDataWrapper).aheadTime;

            float njs;
            float spawnOffset;
            if (customDataWrapper->value) {
                rapidjson::Value &customData = *customDataWrapper->value;
                njs = customData.HasMember("_noteJumpMovementSpeed")
                          ? customData["_noteJumpMovementSpeed"].GetFloat()
                          : CachedNoteJumpMovementSpeed;
                spawnOffset =
                    customData.HasMember("_noteJumpStartBeatOffset")
                        ? customData["_noteJumpStartBeatOffset"].GetFloat()
                        : CachedNoteJumpStartBeatOffset;
            } else {
                njs = CachedNoteJumpMovementSpeed;
                spawnOffset = CachedNoteJumpStartBeatOffset;
            }

            float num = 60 / bpm;
            float num2 = startHalfJumpDurationInBeats;
            while (njs * num * num2 > maxHalfJumpDistance) {
                num /= 2;
            }

            num2 += spawnOffset;
            if (num2 < 1) {
                num2 = 1;
            }

            float jumpDuration = num * num2 * 2;
            *aheadTime = moveDuration + (jumpDuration * 0.5f);
        }

        beatmapLineData->beatmapObjectsData =
            OrderObjects(beatmapLineData->beatmapObjectsData);
    }

    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
}

MAKE_HOOK_MATCH(CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData,
                IReadonlyBeatmapData *, IReadonlyBeatmapData *beatmapData,
                IPreviewBeatmapLevel *beatmapLevel,
                GameplayModifiers *gameplayModifiers,
                PracticeSettings *practiceSettings, bool leftHanded,
                EnvironmentEffectsFilterPreset environmentEffectsFilterPreset,
                EnvironmentIntensityReductionOptions
                    *environmentIntensityReductionOptions) {
    auto transformedBeatmapData = ReorderLineData(beatmapData);
    return CreateTransformedBeatmapData(
        beatmapData, beatmapLevel, gameplayModifiers, practiceSettings,
        leftHanded, environmentEffectsFilterPreset,
        environmentIntensityReductionOptions);
}

void InstallBeatmapDataTransformHelperHooks(Logger &logger) {
    INSTALL_HOOK(logger, CreateTransformedBeatmapData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);