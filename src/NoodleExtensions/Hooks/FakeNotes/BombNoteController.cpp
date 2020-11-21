#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BombNoteController.hpp"

#include "NoodleExtensions/NEHooks.h"
#include "NoodleExtensions/FakeNoteHelper.h"
#include "CustomJSONData/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_OFFSETLESS(BombNoteController_Init, void, BombNoteController *self, NoteData *noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity) {
    if (FakeNoteHelper::GetCuttable(noteData)) {
        BombNoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity);
    }
}

void NoodleExtensions::InstallBombNoteControllerHooks() {
    INSTALL_HOOK_OFFSETLESS(BombNoteController_Init, il2cpp_utils::FindMethodUnsafe("", "BombNoteController", "Init", 8));
}