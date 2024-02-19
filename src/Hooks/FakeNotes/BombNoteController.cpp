#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/CuttableBySaber.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BombNoteController_Init, &BombNoteController::Init, void, BombNoteController* self, NoteData* noteData,
                float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos,
                UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity) {
  BombNoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration,
                          jumpDuration, jumpGravity);
  if (!Hooks::isNoodleHookEnabled()) return;

  if (!FakeNoteHelper::GetCuttable(noteData)) {
    self->_cuttableBySaber->set_canBeCut(false);
  }
}

void InstallBombNoteControllerHooks(Logger& logger) {
  INSTALL_HOOK(logger, BombNoteController_Init);
}
NEInstallHooks(InstallBombNoteControllerHooks);