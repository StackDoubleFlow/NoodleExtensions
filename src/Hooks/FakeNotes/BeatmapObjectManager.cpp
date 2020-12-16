#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"

#include "NEHooks.h"
#include "FakeNoteHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(HandleNoteWasMissed, void, BeatmapObjectManager *self, NoteController *noteController) {
    if (!FakeNoteHelper::GetFakeNote(noteController)) {
        HandleNoteWasMissed(self, noteController);
    }
}

void NoodleExtensions::InstallBeatmapObjectManagerHooks() {
    INSTALL_HOOK_OFFSETLESS(HandleNoteWasMissed, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectManager", "HandleNoteWasMissed", 1));
}