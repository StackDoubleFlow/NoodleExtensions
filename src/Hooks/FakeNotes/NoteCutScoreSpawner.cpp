#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteCutScoreSpawner.hpp"
#include "GlobalNamespace/BadNoteCutEffectSpawner.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(NoteCutScoreSpawner_HandleNoteWasCut, &NoteCutScoreSpawner::HandleNoteWasCut, void,
                NoteCutScoreSpawner *self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    if (!Hooks::isNoodleHookEnabled())
        return NoteCutScoreSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);

    if (!FakeNoteHelper::GetFakeNote(noteController->noteData)) {
        NoteCutScoreSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);
    }
}

void InstallNoteCutScoreSpawnerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutScoreSpawner_HandleNoteWasCut);
}
NEInstallHooks(InstallNoteCutScoreSpawnerHooks);