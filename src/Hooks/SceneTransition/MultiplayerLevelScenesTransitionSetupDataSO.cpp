#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"

#include "Animation/ParentObject.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"

using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;
using namespace NoodleExtensions;

MAKE_HOOK_MATCH(MultiplayerLevelScenesTransitionSetupDataSO_Init,
                &MultiplayerLevelScenesTransitionSetupDataSO::Init,
                void,MultiplayerLevelScenesTransitionSetupDataSO* self,
                StringW gameMode, GlobalNamespace::IPreviewBeatmapLevel* previewBeatmapLevel,
                GlobalNamespace::BeatmapDifficulty beatmapDifficulty, GlobalNamespace::BeatmapCharacteristicSO* beatmapCharacteristic,
                GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap, GlobalNamespace::ColorScheme* overrideColorScheme,
                GlobalNamespace::GameplayModifiers* gameplayModifiers, GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                GlobalNamespace::PracticeSettings* practiceSettings, bool useTestNoteCutSoundEffects) {
    MultiplayerLevelScenesTransitionSetupDataSO_Init(self, gameMode, previewBeatmapLevel,beatmapDifficulty, beatmapCharacteristic,
                                                     difficultyBeatmap,
                                                     overrideColorScheme, gameplayModifiers,
                                                     playerSpecificSettings, practiceSettings, useTestNoteCutSoundEffects);

    std::optional<GlobalNamespace::CustomDifficultyBeatmap *> customBeatmapDataCustomOpt = il2cpp_utils::try_cast<GlobalNamespace::CustomDifficultyBeatmap >(difficultyBeatmap);
    if (customBeatmapDataCustomOpt) {
        NELogger::GetLogger().debug("CustomDifficultyBeatmap casted");
        auto customBeatmapDataCustom = il2cpp_utils::cast<CustomJSONData::v3::CustomBeatmapSaveData>(
                customBeatmapDataCustomOpt.value()->beatmapSaveData);

        SceneTransitionHelper::Patch(difficultyBeatmap,
                                     customBeatmapDataCustom, playerSpecificSettings);
    } else {
        NELogger::GetLogger().debug("CustomDifficultyBeatmap not casted");
        SceneTransitionHelper::Patch(nullptr,
                                     nullptr, playerSpecificSettings);
    }

//    SceneTransitionHelper::Patch(difficultyBeatmap,
//                                 static_cast<CustomBeatmapData *>(difficultyBeatmap->get_beatmapData()), playerSpecificSettings);
}

void InstallMultiplayerLevelScenesTransitionSetupDataSOHooks(Logger& logger) {
    INSTALL_HOOK(logger, MultiplayerLevelScenesTransitionSetupDataSO_Init);
}
NEInstallHooks(InstallMultiplayerLevelScenesTransitionSetupDataSOHooks);