#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(HandleNoteWasSpawned,
                &NoteCutSoundEffectManager::HandleNoteWasSpawned, void,
                NoteCutSoundEffectManager *self,
                NoteController *noteController) {
    // if (!FakeNoteHelper::GetFakeNote(noteController)) {
    //     HandleNoteWasSpawned(self, noteController);
    // }
}

void InstallNoteCutSoundEffectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, HandleNoteWasSpawned);
}
NEInstallHooks(InstallNoteCutSoundEffectManagerHooks);