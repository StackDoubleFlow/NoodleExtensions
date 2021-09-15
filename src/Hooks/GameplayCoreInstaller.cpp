#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultyMethods.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"

#include "NECaches.h"
#include "NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(InstallBindings, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller *self) {
    IDifficultyBeatmap *difficultyBeatmap = self->sceneSetupData->difficultyBeatmap;
    GameplayModifiers *gameplayModifiers = self->sceneSetupData->gameplayModifiers;

    float njs = difficultyBeatmap->get_noteJumpMovementSpeed();
    if (njs <= 0) {
        njs = BeatmapDifficultyMethods::NoteJumpMovementSpeed(difficultyBeatmap->get_difficulty());
    }
    if (gameplayModifiers->fastNotes) {
        njs = 20;
    }
    NECaches::noteJumpMovementSpeed = njs;

    NECaches::noteJumpStartBeatOffset = difficultyBeatmap->get_noteJumpStartBeatOffset() + self->sceneSetupData->playerSpecificSettings->noteJumpStartBeatOffset;

    InstallBindings(self);
}

void InstallGameplayCoreInstallerHooks(Logger& logger) {
    INSTALL_HOOK(logger, InstallBindings);
}

NEInstallHooks(InstallGameplayCoreInstallerHooks);