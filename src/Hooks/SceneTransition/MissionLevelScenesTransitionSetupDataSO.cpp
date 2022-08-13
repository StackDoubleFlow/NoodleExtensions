#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/MissionLevelScenesTransitionSetupDataSO.hpp"
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

MAKE_HOOK_MATCH(MissionLevelScenesTransitionSetupDataSO_Init, &MissionLevelScenesTransitionSetupDataSO::Init, void, MissionLevelScenesTransitionSetupDataSO *self,
                StringW missionId, GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap,
                GlobalNamespace::IPreviewBeatmapLevel* previewBeatmapLevel, ::ArrayW<GlobalNamespace::MissionObjective*> missionObjectives,
                GlobalNamespace::ColorScheme* overrideColorScheme, GlobalNamespace::GameplayModifiers* gameplayModifiers,
                GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings, StringW backButtonText) {
    MissionLevelScenesTransitionSetupDataSO_Init(self, missionId, difficultyBeatmap, previewBeatmapLevel, missionObjectives, overrideColorScheme, gameplayModifiers, playerSpecificSettings, backButtonText);

//    SceneTransitionHelper::Patch(difficultyBeatmap,
//                                 static_cast<CustomBeatmapData *>(difficultyBeatmap->get_beatmapData()), playerSpecificSettings);
}

void InstallMissionLevelScenesTransitionSetupDataSOHooks(Logger& logger) {
    INSTALL_HOOK(logger, MissionLevelScenesTransitionSetupDataSO_Init);
}
NEInstallHooks(InstallMissionLevelScenesTransitionSetupDataSOHooks);