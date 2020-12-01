#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/IDifficultyBeatmap.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultyMethods.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;

int CachedNoteJumpMovementSpeed;
int CachedNoteJumpStartBeatOffset;

MAKE_HOOK_OFFSETLESS(InstallBindings, void, GameplayCoreInstaller *self) {
    IDifficultyBeatmap *difficultyBeatmap = self->sceneSetupData->difficultyBeatmap;
    GameplayModifiers *gameplayModifiers = self->sceneSetupData->gameplayModifiers;

    float njs = difficultyBeatmap->get_noteJumpMovementSpeed();
    if (njs <= 0) {
        njs = BeatmapDifficultyMethods::NoteJumpMovementSpeed(difficultyBeatmap->get_difficulty());
    }
    if (gameplayModifiers->fastNotes) {
        njs = 20;
    }
    CachedNoteJumpMovementSpeed = njs;

    CachedNoteJumpStartBeatOffset = difficultyBeatmap->get_noteJumpStartBeatOffset() + self->sceneSetupData->playerSpecificSettings->noteJumpStartBeatOffset;

    InstallBindings(self);
}

void NoodleExtensions::InstallGameplayCoreInstallerHooks() {
    INSTALL_HOOK_OFFSETLESS(InstallBindings, il2cpp_utils::FindMethodUnsafe("", "GameplayCoreInstaller", "InstallBindings", 0));
}