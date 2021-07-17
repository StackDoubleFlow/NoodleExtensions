#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData *noteUpdateAD;

float noteTimeAdjust(float original, float jumpDuration);

MAKE_HOOK_MATCH(NoteJump_ManualUpdate, &NoteJump::ManualUpdate, Vector3, NoteJump *self) {
    Vector3 result = NoteJump_ManualUpdate(self);

    if (noteUpdateAD) {
        float songTime = self->audioTimeSyncController->get_songTime();
		float elapsedTime = songTime - (self->beatTime - self->jumpDuration * 0.5f);
        elapsedTime = noteTimeAdjust(elapsedTime, self->jumpDuration);
		float normalTime = elapsedTime / self->jumpDuration;
        std::optional<Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteUpdateAD->track, normalTime);
        if (position.has_value()) {
            self->localPosition = *position + noteUpdateAD->noteOffset;
            result = self->worldRotation * self->localPosition;
            self->get_transform()->set_localPosition(result);
        }
    }

    return result;
} 

void InstallNoteJumpHooks(Logger& logger) {
    INSTALL_HOOK(logger, NoteJump_ManualUpdate);
}

NEInstallHooks(InstallNoteJumpHooks);
