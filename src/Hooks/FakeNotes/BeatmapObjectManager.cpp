#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"

#include "NEHooks.h"
#include "FakeNoteHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;


MAKE_HOOK_OFFSETLESS(HandleNoteControllerNoteWasMissed, void, BeatmapObjectManager *self, NoteController *noteController) {
    // NELogger::GetLogger().info("noteData pointer %p", noteController->noteData);
    // NELogger::GetLogger().info("Checking if fake note");
    if (!FakeNoteHelper::GetFakeNote(noteController)) {
        HandleNoteControllerNoteWasMissed(self, noteController);
    }
}

void NoodleExtensions::InstallBeatmapObjectManagerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, HandleNoteControllerNoteWasMissed, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectManager", "HandleNoteControllerNoteWasMissed", 1));
}