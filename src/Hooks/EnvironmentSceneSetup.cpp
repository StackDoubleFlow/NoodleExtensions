#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/EnvironmentSceneSetup.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultyMethods.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnControllerHelpers.hpp"
#include "GlobalNamespace/IBeatmapLevel.hpp"
#include "GlobalNamespace/IPreviewBeatmapLevel.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/IBeatmapDataBasicInfo.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteJumpValueType.hpp"
#include "UnityEngine/Shader.hpp"

#include "Zenject/DiContainer.hpp"
#include "NECaches.h"
#include "NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(EnvironmentSceneSetup_InstallBindings, &EnvironmentSceneSetup::InstallBindings, void,
                EnvironmentSceneSetup* self) {
  EnvironmentSceneSetup_InstallBindings(self);

  if (!Hooks::isNoodleHookEnabled()) return;

  static int const _trackLaneYPositionPropertyId = UnityEngine::Shader::PropertyToID("_TrackLaneYPosition");

  UnityEngine::Shader::SetGlobalFloat(_trackLaneYPositionPropertyId, -1000000.0f);
}

void InstallEnvironmentSceneSetupHooks(Logger& logger) {
  INSTALL_HOOK(logger, EnvironmentSceneSetup_InstallBindings);
}

NEInstallHooks(InstallEnvironmentSceneSetupHooks);