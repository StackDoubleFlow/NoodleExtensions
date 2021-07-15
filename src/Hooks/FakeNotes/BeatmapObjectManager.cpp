#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(HandleNoteControllerNoteWasMissed,
                &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void,
                BeatmapObjectManager *self, NoteController *noteController) {
    if (!FakeNoteHelper::GetFakeNote(noteController)) {
        HandleNoteControllerNoteWasMissed(self, noteController);
    }
}

void InstallBeatmapObjectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, HandleNoteControllerNoteWasMissed);
}

NEInstallHooks(InstallBeatmapObjectManagerHooks);