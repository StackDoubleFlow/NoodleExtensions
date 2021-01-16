#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"

#include "NEHooks.h"
#include "FakeNoteHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(HandleNoteWasSpawned, void, NoteCutSoundEffectManager *self, NoteController *noteController) {
    // if (!FakeNoteHelper::GetFakeNote(noteController)) {
    //     HandleNoteWasSpawned(self, noteController);
    // }
}

void NoodleExtensions::InstallNoteCutSoundEffectManagerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, HandleNoteWasSpawned, il2cpp_utils::FindMethodUnsafe("", "NoteCutSoundEffectManager", "HandleNoteWasSpawned", 1));
}