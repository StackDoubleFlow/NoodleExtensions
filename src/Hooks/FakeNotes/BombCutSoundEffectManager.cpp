#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BombCutSoundEffectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/BadNoteCutEffectSpawner.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BombCutSoundEffectManager_HandleNoteWasCut, &BombCutSoundEffectManager::HandleNoteWasCut, void,
                BombCutSoundEffectManager *self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    if (!Hooks::isNoodleHookEnabled())
        return BombCutSoundEffectManager_HandleNoteWasCut(self, noteController, noteCutInfo);

    if (!FakeNoteHelper::GetFakeNote(noteController->noteData)) {
        BombCutSoundEffectManager_HandleNoteWasCut(self, noteController, noteCutInfo);
    }
}

void InstallBombCutSoundEffectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, BombCutSoundEffectManager_HandleNoteWasCut);
}
NEInstallHooks(InstallBombCutSoundEffectManagerHooks);