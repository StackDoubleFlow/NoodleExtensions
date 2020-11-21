#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/GameNoteController.hpp"

#include "NoodleExtensions/NEHooks.h"
#include "NoodleExtensions/FakeNoteHelper.h"
#include "CustomJSONData/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(NoteDidStartJump, void, GameNoteController *self) {
    if (FakeNoteHelper::GetCuttable(self->noteData)) {
        NoteDidStartJump(self);
    }
}

void NoodleExtensions::InstallGameNoteControllerHooks() {
    INSTALL_HOOK_OFFSETLESS(NoteDidStartJump, il2cpp_utils::FindMethodUnsafe("", "GameNoteController", "NoteDidStartJump", 0));
}