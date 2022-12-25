#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"

#include "Animation/ParentObject.h"
#include "tracks/shared/Animation/PointDefinition.h"
#include "AssociatedData.h"
#include "NECaches.h"
#include "NEConfig.h"
#include "NELogger.h"
#include "NEHooks.h"
#include "SceneTransitionHelper.hpp"

#include "pinkcore/shared/RequirementAPI.hpp"

// // needed to compile, idk why
// #define ID "Noodle"
// #include "conditional-dependencies/shared/main.hpp"

// #include "qosmetics-api/shared/WallAPI.hpp"
// #include "qosmetics-api/shared/NoteAPI.hpp"

// #undef ID

#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"

using namespace NoodleExtensions;
using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;

void SceneTransitionHelper::Patch(IDifficultyBeatmap *difficultyBeatmap, CustomJSONData::v3::CustomBeatmapSaveData *customBeatmapDataCustom, PlayerSpecificSettings *playerSpecificSettings) {
    NECaches::LeftHandedMode = playerSpecificSettings->leftHanded;
    bool noodleRequirement = false;
    bool meRequirement = false;

    if (customBeatmapDataCustom && customBeatmapDataCustom->levelCustomData) {
        auto dynData = customBeatmapDataCustom->levelCustomData;

        if (dynData && dynData->get().IsObject()) {
            ValueUTF16 const& rapidjsonData = *dynData;

            meRequirement = CheckIfME(rapidjsonData);
            noodleRequirement = CheckIfNoodle(rapidjsonData);
        }
    }

    noodleRequirement = !meRequirement && noodleRequirement;

    Hooks::setNoodleHookEnabled(noodleRequirement);

    auto const& modInfo = NELogger::modInfo;
    // if (noodleRequirement && getNEConfig().qosmeticsModelDisable.GetValue()) {
    //     Qosmetics::NoteAPI::RegisterNoteDisablingInfo(modInfo);
    //     Qosmetics::WallAPI::RegisterWallDisablingInfo(modInfo);
    // } else {
    //     Qosmetics::NoteAPI::UnregisterNoteDisablingInfo(modInfo);
    //     Qosmetics::WallAPI::UnregisterWallDisablingInfo(modInfo);
    // }

    ParentController::OnDestroy();

    static auto *customObstacleDataClass = classof(CustomJSONData::CustomObstacleData *);
    static auto *customNoteDataClass = classof(CustomJSONData::CustomNoteData *);
//
//    if (difficultyBeatmap) {
//        auto *beatmapData = reinterpret_cast<CustomBeatmapData *>(difficultyBeatmap->get_beatmapData());
//
//        for (BeatmapLineData *beatmapLineData : beatmapData->beatmapLinesData) {
//            if (!beatmapLineData)
//                continue;
//
//            for (int j = 0; j < beatmapLineData->beatmapObjectsData->size; j++) {
//                BeatmapObjectData *beatmapObjectData =
//                        beatmapLineData->beatmapObjectsData->items.get(j);
//
//                CustomJSONData::JSONWrapper *customDataWrapper;
//                if (beatmapObjectData->klass == customObstacleDataClass) {
//                    auto obstacleData =
//                            (CustomJSONData::CustomObstacleData *) beatmapObjectData;
//                    customDataWrapper = obstacleData->customData;
//                } else if (beatmapObjectData->klass == customNoteDataClass) {
//                    auto noteData =
//                            (CustomJSONData::CustomNoteData *) beatmapObjectData;
//                    customDataWrapper = noteData->customData;
//                } else {
//                    continue;
//                }
//
//                if (customDataWrapper) {
//                    BeatmapObjectAssociatedData &ad = getAD(customDataWrapper);
//                    ad.ResetState();
//                }
//            }
//        }
//    }

    clearEventADs();
    NECaches::ClearNoteCaches();
    NECaches::ClearObstacleCaches();
}
