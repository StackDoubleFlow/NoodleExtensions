#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData *noteUpdateAD;

MAKE_HOOK_OFFSETLESS(NoteJump_ManualUpdate, Vector3, NoteJump *self) {
    Vector3 result = NoteJump_ManualUpdate(self);

    if (noteUpdateAD) {
        float songTime = self->audioTimeSyncController->get_songTime();
		float num = songTime - (self->beatTime - self->jumpDuration * 0.5f);
		float num2 = num / self->jumpDuration;
        std::optional<Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteUpdateAD->track, num2);
        if (position.has_value()) {
            self->localPosition = *position + noteUpdateAD->noteOffset;
            result = self->worldRotation * self->localPosition;
            self->get_transform()->set_localPosition(result);
        }
    }

    return result;
} 

void InstallNoteJumpHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, NoteJump_ManualUpdate, il2cpp_utils::FindMethodUnsafe("", "NoteJump", "ManualUpdate", 0));
}

NEInstallHooks(InstallNoteJumpHooks);
