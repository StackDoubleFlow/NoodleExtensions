#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

extern std::optional<NoteCutCoreEffectsSpawner*> noteCutCoreEffectsSpawner;

MAKE_HOOK_MATCH(HandleNoteControllerNoteWasMissed,
                &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void,
                BeatmapObjectManager *self, NoteController *noteController) {
    if (!FakeNoteHelper::GetFakeNote(noteController)) {
        HandleNoteControllerNoteWasMissed(self, noteController);
    }
}

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut,
                &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void,
                BeatmapObjectManager *self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    if (FakeNoteHelper::GetFakeNote(noteController) && noteCutCoreEffectsSpawner) {
        CRASH_UNLESS(*noteCutCoreEffectsSpawner);
        noteCutCoreEffectsSpawner.value()->HandleNoteWasCut(noteController, noteCutInfo);
        self->Despawn(noteController);
        return;

    }
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);
}

void InstallBeatmapObjectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, HandleNoteControllerNoteWasMissed);
//    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
}

NEInstallHooks(InstallBeatmapObjectManagerHooks);