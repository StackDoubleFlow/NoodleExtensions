#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "UnityEngine/Transform.hpp"
#include "System/Action.hpp"

#include "Animation/AnimationHelper.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "NEHooks.h"
#include "NELogger.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace System;

extern BeatmapObjectAssociatedData *noteUpdateAD;
extern Track *noteTrack;

MAKE_HOOK_MATCH(NoteFloorMovement_ManualUpdate, &NoteFloorMovement::ManualUpdate, Vector3, NoteFloorMovement *self) {
    float num = TimeSourceHelper::getSongTime(self->audioTimeSyncController) - self->startTime;

    Vector3 localPosition = Vector3::Lerp(self->startPos, self->endPos, num / self->moveDuration);
    if (noteUpdateAD) {
        std::optional<Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTrack, 0);
        if (position.has_value()) {
            Vector3 noteOffset = noteUpdateAD->noteOffset;
            Vector3 endPos = self->endPos;
            localPosition = localPosition + (*position + noteOffset - endPos);
        }
    }
    self->localPosition = localPosition;
    
    
    Vector3 vector = self->worldRotation * self->localPosition;
    self->get_transform()->set_localPosition(vector);
    if (num >= self->moveDuration) {
        Action *action = self->floorMovementDidFinishEvent;
        if (action) {
            action->Invoke();
        }
    }
    return vector; 
}

MAKE_HOOK_MATCH(NoteFloorMovement_SetToStart, &NoteFloorMovement::SetToStart, UnityEngine::Vector3, NoteFloorMovement *self) {
    auto ret = NoteFloorMovement_SetToStart(self);

    if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
        self->get_transform()->set_localRotation(NEVector::Quaternion::Euler({0, 0, noteUpdateAD->endRotation}));
    }

    return ret;
}

void InstallNoteFloorMovementHooks(Logger& logger) {
    INSTALL_HOOK_ORIG(logger, NoteFloorMovement_ManualUpdate);
    INSTALL_HOOK(logger, NoteFloorMovement_SetToStart);
}

NEInstallHooks(InstallNoteFloorMovementHooks);
