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
extern TracksAD::TracksVector noteTracks;

static NEVector::Vector3 DefinitePositionTranspile(NEVector::Vector3 original, NoteFloorMovement* noteFloorMovement) {
    if (!noteUpdateAD) {
        return original;
    }

    std::optional<NEVector::Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData,
                                                                                           noteTracks, 0);
    if (!position.has_value()) {
        return original;
    }

    NEVector::Vector3 noteOffset = noteUpdateAD->noteOffset;
    NEVector::Vector3 endPos = noteFloorMovement->endPos;
    return original + (position.value() + noteOffset - endPos);
}

MAKE_HOOK_MATCH(NoteFloorMovement_ManualUpdate, &NoteFloorMovement::ManualUpdate, Vector3, NoteFloorMovement *self) {
    if (!Hooks::isNoodleHookEnabled())
        return NoteFloorMovement_ManualUpdate(self);
    float num = TimeSourceHelper::getSongTime(self->audioTimeSyncController) - self->startTime;

    self->localPosition = NEVector::Vector3::Lerp(self->startPos, self->endPos, num / self->moveDuration);
    self->localPosition = DefinitePositionTranspile(self->localPosition, self);

    NEVector::Vector3 vector = NEVector::Quaternion(self->worldRotation) * NEVector::Vector3(self->localPosition);
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
    if (!Hooks::isNoodleHookEnabled())
        return NoteFloorMovement_SetToStart(self);

    auto ret = NoteFloorMovement_SetToStart(self);

    if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
        self->rotatedObject->set_localRotation(NEVector::Quaternion::Euler({0, 0, noteUpdateAD->endRotation}));
    }

    return ret;
}

void InstallNoteFloorMovementHooks(Logger& logger) {
    INSTALL_HOOK_ORIG(logger, NoteFloorMovement_ManualUpdate);
    INSTALL_HOOK(logger, NoteFloorMovement_SetToStart);
}

NEInstallHooks(InstallNoteFloorMovementHooks);
