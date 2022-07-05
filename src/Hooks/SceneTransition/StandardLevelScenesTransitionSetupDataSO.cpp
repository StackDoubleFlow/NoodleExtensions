#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/CustomDifficultyBeatmap.hpp"

#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"

#include "Animation/ParentObject.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"

using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;
using namespace NoodleExtensions;

MAKE_HOOK_MATCH(StandardLevelScenesTransitionSetupDataSO_Init, &StandardLevelScenesTransitionSetupDataSO::Init, void, StandardLevelScenesTransitionSetupDataSO *self,
                StringW gameMode, ::GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap, ::GlobalNamespace::IPreviewBeatmapLevel* previewBeatmapLevel,
                ::GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings, ::GlobalNamespace::ColorScheme* overrideColorScheme,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers, ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::PracticeSettings* practiceSettings, ::StringW backButtonText, bool useTestNoteCutSoundEffects,
                bool startPaused) {
    StandardLevelScenesTransitionSetupDataSO_Init(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects, startPaused);

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
}

void InstallStandardLevelScenesTransitionSetupDataSOHooks(Logger& logger) {
    INSTALL_HOOK(logger, StandardLevelScenesTransitionSetupDataSO_Init);
}
NEInstallHooks(InstallStandardLevelScenesTransitionSetupDataSOHooks);