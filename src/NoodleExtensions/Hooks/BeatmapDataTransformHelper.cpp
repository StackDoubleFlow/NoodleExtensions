#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "System/Func_2.hpp"
#include "System/Linq/Enumerable.hpp"
#include "System/Collections/Generic/IEnumerable_1.hpp"
#include "System/Linq/IOrderedEnumerable_1.hpp"

#include "CustomJSONData/CustomBeatmapData.h"
#include "NoodleExtensions/NEHooks.h"
#include "NELogger.h"

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

extern int CachedNoteJumpMovementSpeed;
extern int CachedNoteJumpStartBeatOffset;

Il2CppClass *customObstacleDataClass;
Il2CppClass *customNoteDataClass;

System::Func_2<BeatmapObjectData *, float> *CreateOrderFunc() {
    std::vector<const Il2CppClass *> argClasses{ classof(BeatmapObjectData *), classof(float) };
    auto genericClass = il2cpp_utils::MakeGeneric(il2cpp_utils::GetClassFromName("System", "Func`2"), argClasses);
    auto lambda = +[](BeatmapObjectData *n) {
        if (n->klass == customObstacleDataClass) {
            return n->time - ((CustomJSONData::CustomObstacleData *) n)->aheadTime;
        } else if (n->klass == customNoteDataClass) {
            return n->time - ((CustomJSONData::CustomNoteData *) n)->aheadTime;
        } else {
            return n->time;
        }
    };
    return il2cpp_utils::MakeDelegate<System::Func_2<BeatmapObjectData *, float> *>(genericClass, static_cast<Il2CppObject*>(nullptr), lambda);
}

List<BeatmapObjectData *> *OrderObjects(List<BeatmapObjectData *> *beatmapObjectsData) {
    auto orderFunc = CreateOrderFunc();

    using IOrderedEnumerableT = System::Linq::IOrderedEnumerable_1<BeatmapObjectData*>*;
    // Used as a parameter for OrderBy
    auto enumerable = (IEnumerable_1<BeatmapObjectData*> *) beatmapObjectsData;
    std::vector<Il2CppClass *> genericParams = {classof(BeatmapObjectData*), classof(float)};
    auto orderByMethodInfo = il2cpp_utils::FindMethodUnsafe("System.Linq", "Enumerable", "OrderBy", 2);
    auto orderByGenericMethodInfo = il2cpp_utils::MakeGenericMethod(orderByMethodInfo, genericParams);
    auto orderedEnumerable = CRASH_UNLESS(il2cpp_utils::RunMethod<IEnumerable_1<BeatmapObjectData*> *>(nullptr, orderByGenericMethodInfo, enumerable, orderFunc));
    
    auto toListMethodInfo = il2cpp_utils::FindMethodUnsafe("System.Linq", "Enumerable", "ToList", 1);
    auto toListGenericMethodInfo = il2cpp_utils::MakeGenericMethod(toListMethodInfo, {classof(BeatmapObjectData *)});
    return CRASH_UNLESS(il2cpp_utils::RunMethod<List<BeatmapObjectData*>*>(nullptr, toListGenericMethodInfo, orderedEnumerable));

    // auto arr = beatmapObjectsData->items->values;
    // std::sort(arr, arr + beatmapObjectsData->items->Length(), [](BeatmapObjectData *a, BeatmapObjectData *b) {
    //     NELogger::GetLogger().debug("Comparing %p with %p", a, b);
    //     __android_log_print(ANDROID_LOG_DEBUG, "NOODLEEXTENSIONS", "Comparing %p with %p", a, b);
    //     usleep(100);
    //     float aTime = a->time - CRASH_UNLESS(il2cpp_utils::GetFieldValue<float>(a, "aheadTime"));
    //     float bTime = b->time - CRASH_UNLESS(il2cpp_utils::GetFieldValue<float>(b, "aheadTime"));
    //     return aTime < bTime;
    // });
    // return beatmapObjectsData;
}


IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
    BeatmapData *customBeatmapData = beatmapData->GetCopy();

    float startHalfJumpDurationInBeats = 4;
    float maxHalfJumpDistance = 18;
    float moveDuration = 0.5f;

    if (!customObstacleDataClass) {
        customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
        customNoteDataClass = classof(CustomJSONData::CustomNoteData *);
    }

    // loop through all objects in all lines of the beatmapData
    for (int i = 0; i < customBeatmapData->beatmapLinesData->Length(); i++) {
        BeatmapLineData *beatmapLineData = customBeatmapData->beatmapLinesData->values[i]; 
        for (int j = 0; j < beatmapLineData->beatmapObjectsData->get_Count(); j++) {
            BeatmapObjectData *beatmapObjectData = beatmapLineData->beatmapObjectsData->items->values[j];
            float bpm;
            float *aheadTime;

            CustomJSONData::JSONWrapper *customDataWrapper;
            if (beatmapObjectData->klass == customObstacleDataClass) {
                NELogger::GetLogger().info("found a CustomObstacleData in transform");
                auto obstacleData = (CustomJSONData::CustomObstacleData *) beatmapObjectData;
                customDataWrapper = obstacleData->customData;
                bpm = obstacleData->bpm;
                aheadTime = &obstacleData->aheadTime;
            } else if (beatmapObjectData->klass == customNoteDataClass) {
                auto noteData = (CustomJSONData::CustomNoteData *) beatmapObjectData;
                customDataWrapper = noteData->customData;
                bpm = noteData->bpm;
                aheadTime = &noteData->aheadTime;
            } else {
                continue;
            }

            float njs;
            float spawnOffset;
            if (customDataWrapper) {
                NELogger::GetLogger().info("This one has custom data");
                rapidjson::Value &customData = *customDataWrapper->value;
                njs = customData.HasMember("_noteJumpMovementSpeed") ? customData["_noteJumpMovementSpeed"].GetFloat() : CachedNoteJumpMovementSpeed;
                spawnOffset = customData.HasMember("_noteJumpStartBeatOffset") ? customData["_noteJumpStartBeatOffset"].GetFloat() : CachedNoteJumpStartBeatOffset;
            } else {
                NELogger::GetLogger().info("This one doesn't have custom data");
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

        beatmapLineData->beatmapObjectsData = OrderObjects(beatmapLineData->beatmapObjectsData);
    }

    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
}

MAKE_HOOK_OFFSETLESS(CreateTransformedBeatmapData, IReadonlyBeatmapData *, IReadonlyBeatmapData *beatmapData, GameplayModifiers *gameplayModifiers, PracticeSettings *practiceSettings, bool leftHanded, bool staticLights) {
    auto transformedBeatmapData = ReorderLineData(beatmapData);
    return CreateTransformedBeatmapData(transformedBeatmapData, gameplayModifiers, practiceSettings, leftHanded, staticLights);
}

void NoodleExtensions::InstallBeatmapDataTransformHelperHooks() {
    INSTALL_HOOK_OFFSETLESS(CreateTransformedBeatmapData, il2cpp_utils::FindMethodUnsafe("", "BeatmapDataTransformHelper", "CreateTransformedBeatmapData", 5));
}