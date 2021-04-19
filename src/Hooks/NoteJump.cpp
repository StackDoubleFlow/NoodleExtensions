#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteJump.hpp"

#include "Animation/AnimationHelper.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData *noteUpdateAD;

MAKE_HOOK_OFFSETLESS(NoteJump_ManualUpdate, Vector3, NoteJump *self) {
    // Vector3 localPosition = self->localPosition;
    // if (noteUpdateAD) {
    //     std::optional<Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteUpdateAD->track, 0);
    //     if (position.has_value()) {
    //         Vector3 noteOffset = noteUpdateAD->noteOffset;
    //         Vector3 endPos = self->endPos;
    //         localPosition = localPosition + (*position + noteOffset - endPos);
    //     }
    // }
    // self->localPosition = localPosition;
}

void NoodleExtensions::InstallNoteJumpHooks(Logger& logger) {
    // INSTALL_HOOK_OFFSETLESS(logger, NoteJump_ManualUpdate, il2cpp_utils::FindMethodUnsafe("", "NoteJump", "ManualUpdate", 0));
}
