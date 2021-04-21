#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "GlobalNamespace/StandardLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/OverrideEnvironmentSettings.hpp"
#include "GlobalNamespace/ColorScheme.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"

#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;

MAKE_HOOK_OFFSETLESS(StandardLevelScenesTransitionSetupDataSO_Init, void, StandardLevelScenesTransitionSetupDataSO *self, Il2CppString *gameMode, IDifficultyBeatmap *difficultyBeatmap, IPreviewBeatmapLevel *previewBeatmapLevel, OverrideEnvironmentSettings *overrideEnvironmentSettings, ColorScheme *overrideColorScheme, GameplayModifiers *gameplayModifiers, PlayerSpecificSettings *playerSpecificSettings, PracticeSettings *practiceSettings, Il2CppString *backButtonText, bool useTestNoteCutSoundEffects) {
    StandardLevelScenesTransitionSetupDataSO_Init(self, gameMode, difficultyBeatmap, previewBeatmapLevel, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
    ParentController::OnDestroy();

    auto *beatmapData = reinterpret_cast<CustomBeatmapData*>(difficultyBeatmap->get_beatmapData());
    auto& ad = getBeatmapAD(beatmapData->customData);
    for (auto& pair : ad.tracks) {
        // TODO: Make this work
        // pair.second.ResetVariables();
    }
}

void NoodleExtensions::InstallStandardLevelScenesTransitionSetupDataSOHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, StandardLevelScenesTransitionSetupDataSO_Init, 
        il2cpp_utils::FindMethodUnsafe("", "StandardLevelScenesTransitionSetupDataSO", "Init", 10));
}