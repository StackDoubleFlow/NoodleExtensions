#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/GameNoteController.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(NoteDidStartJump, &GameNoteController::NoteDidStartJump, void,
                GameNoteController *self) {
    if (FakeNoteHelper::GetCuttable(self->noteData)) {
        NoteDidStartJump(self);
    }
}

void InstallGameNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteDidStartJump);
}
NEInstallHooks(InstallGameNoteControllerHooks);