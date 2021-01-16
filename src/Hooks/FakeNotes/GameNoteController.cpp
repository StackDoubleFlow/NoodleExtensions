#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/GameNoteController.hpp"

#include "NEHooks.h"
#include "FakeNoteHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(NoteDidStartJump, void, GameNoteController *self) {
    if (FakeNoteHelper::GetCuttable(self->noteData)) {
        NoteDidStartJump(self);
    }
}

void NoodleExtensions::InstallGameNoteControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, NoteDidStartJump, il2cpp_utils::FindMethodUnsafe("", "GameNoteController", "NoteDidStartJump", 0));
}