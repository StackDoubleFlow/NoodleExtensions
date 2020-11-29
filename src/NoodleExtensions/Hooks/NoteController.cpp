#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteMovement.hpp"

#include "CustomJSONData/CustomBeatmapData.h"
#include "NoodleExtensions/NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_OFFSETLESS(NoteController_Init, void, NoteController *self, CustomJSONData::CustomNoteData *noteData, float worldRotation, UnityEngine::Vector3 startPos, UnityEngine::Vector3 midPos, UnityEngine::Vector3 endPos, float move1Duration, float move2Duration, float jumpGravity, float endRotation) {
    NoteController_Init(self, noteData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, jumpGravity, endRotation);
    // self->noteMovement->floorMovement
}

void NoodleExtensions::InstallNoteControllerHooks() {
    // INSTALL_HOOK_OFFSETLESS(NoteController_Init, il2cpp_utils::FindMethodUnsafe("", "NoteController", "Init", 9));
}