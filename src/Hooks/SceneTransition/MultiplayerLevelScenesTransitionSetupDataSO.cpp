#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupDataSO.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"

#include "Animation/ParentObject.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"

using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;
using namespace NoodleExtensions;

MAKE_HOOK_MATCH(MultiplayerLevelScenesTransitionSetupDataSO_Init,
                &MultiplayerLevelScenesTransitionSetupDataSO::InitAndSetupScenes, void,
                MultiplayerLevelScenesTransitionSetupDataSO* self) {

  auto customBeatmapLevel = il2cpp_utils::try_cast<SongCore::SongLoader::CustomBeatmapLevel>(self->get_beatmapLevel());
  if (!customBeatmapLevel) {
    MultiplayerLevelScenesTransitionSetupDataSO_Init(self);
  }

  // TODO: Fix environment override
  SceneTransitionHelper::Patch(customBeatmapLevel.value(), self->beatmapKey,
                               self->GetOrLoadMultiplayerEnvironmentInfo(),
                               self->gameplayCoreSceneSetupData->playerSpecificSettings);

  MultiplayerLevelScenesTransitionSetupDataSO_Init(self);
}

void InstallMultiplayerLevelScenesTransitionSetupDataSOHooks() {
  INSTALL_HOOK(NELogger::Logger, MultiplayerLevelScenesTransitionSetupDataSO_Init);
}
NEInstallHooks(InstallMultiplayerLevelScenesTransitionSetupDataSOHooks);