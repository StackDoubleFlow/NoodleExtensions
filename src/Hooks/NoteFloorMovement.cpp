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
extern std::vector<Track *> noteTracks;

MAKE_HOOK_MATCH(NoteFloorMovement_ManualUpdate, &NoteFloorMovement::ManualUpdate, Vector3, NoteFloorMovement *self) {
    float num = TimeSourceHelper::getSongTime(self->audioTimeSyncController) - self->startTime;

    NEVector::Vector3 localPosition = NEVector::Vector3::Lerp(self->startPos, self->endPos, num / self->moveDuration);
    if (noteUpdateAD) {
        std::optional<NEVector::Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTracks, 0);
        if (position.has_value()) {
            NEVector::Vector3 noteOffset = noteUpdateAD->noteOffset;
            NEVector::Vector3 endPos = self->endPos;
            localPosition = localPosition + (*position + noteOffset - endPos);
        }
    }
    self->localPosition = localPosition;


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
