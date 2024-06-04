#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/BadNoteCutEffectSpawner.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BadNoteCutEffectSpawner_HandleNoteWasCut, &BadNoteCutEffectSpawner::HandleNoteWasCut, void,
                BadNoteCutEffectSpawner* self, GlobalNamespace::NoteController* noteController,
                ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
  if (!Hooks::isNoodleHookEnabled()) return BadNoteCutEffectSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);

  if (!FakeNoteHelper::GetFakeNote(noteController->noteData)) {
    BadNoteCutEffectSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);
  }
}

void InstallBadNoteCutEffectSpawnerHooks() {
  INSTALL_HOOK(NELogger::Logger, BadNoteCutEffectSpawner_HandleNoteWasCut);
}
NEInstallHooks(InstallBadNoteCutEffectSpawnerHooks);