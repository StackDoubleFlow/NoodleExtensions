#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/GameplayCoreInstaller.hpp"
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

#include "Zenject/DiContainer.hpp"

#include "NECaches.h"
#include "NEHooks.h"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteJumpValueType.hpp"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(InstallBindings, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller* self) {
  if (!Hooks::isNoodleHookEnabled()) return InstallBindings(self);

  IDifficultyBeatmap* difficultyBeatmap = self->sceneSetupData->difficultyBeatmap;
  GameplayModifiers* gameplayModifiers = self->sceneSetupData->gameplayModifiers;

  BeatmapObjectSpawnControllerHelpers::GetNoteJumpValues(
      self->sceneSetupData->playerSpecificSettings, difficultyBeatmap->get_noteJumpStartBeatOffset(),
      ByRef(NECaches::noteJumpValueType), ByRef(NECaches::noteJumpValue));

  float njs = difficultyBeatmap->get_noteJumpMovementSpeed();
  if (njs <= 0) {
    njs = BeatmapDifficultyMethods::NoteJumpMovementSpeed(difficultyBeatmap->get_difficulty());
  }
  if (gameplayModifiers->fastNotes) {
    njs = 20;
  }
  NECaches::noteJumpMovementSpeed = njs;

  auto* previewBeatmapLevel = reinterpret_cast<IPreviewBeatmapLevel*>(difficultyBeatmap->get_level());

  NECaches::noteJumpStartBeatOffset = difficultyBeatmap->get_noteJumpStartBeatOffset() +
                                      self->sceneSetupData->playerSpecificSettings->noteJumpStartBeatOffset;
  NECaches::beatsPerMinute = previewBeatmapLevel->get_beatsPerMinute();
  NECaches::numberOfLines =
      reinterpret_cast<IBeatmapDataBasicInfo*>(self->sceneSetupData->transformedBeatmapData)->get_numberOfLines();

  InstallBindings(self);

  NECaches::GameplayCoreContainer = self->get_Container();
}

void InstallGameplayCoreInstallerHooks(Logger& logger) {
  INSTALL_HOOK(logger, InstallBindings);
}

NEInstallHooks(InstallGameplayCoreInstallerHooks);