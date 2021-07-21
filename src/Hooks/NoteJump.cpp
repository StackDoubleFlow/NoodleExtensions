#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/Easing.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/Transform.hpp"
#include "System/Action.hpp"
#include "System/Action_1.hpp"

#include "TimeSourceHelper.h"
#include "Animation/AnimationHelper.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData *noteUpdateAD;

float noteTimeAdjust(float original, float jumpDuration);

MAKE_HOOK_MATCH(NoteJump_ManualUpdate, &NoteJump::ManualUpdate, Vector3, NoteJump *self) {
    float songTime = TimeSourceHelper::getSongTime(self->audioTimeSyncController);
    float elapsedTime = songTime - (self->beatTime - self->jumpDuration * 0.5f);
    if (noteUpdateAD) {
        elapsedTime = noteTimeAdjust(elapsedTime, self->jumpDuration);
    }
    float normalTime = elapsedTime / self->jumpDuration;

    if (self->startPos.x == self->endPos.x) {
        self->localPosition.x = self->startPos.x;
    } else if (normalTime < 0.25) {
        self->localPosition.x = self->startPos.x + (self->endPos.x - self->startPos.x) * Easing::InOutQuad(normalTime * 4);
    } else {
        self->localPosition.x = self->endPos.x;
    }
    self->localPosition.z = self->playerTransforms->MoveTowardsHead(self->startPos.z, self->endPos.z, self->inverseWorldRotation, normalTime);
    self->localPosition.y = self->startPos.y + self->startVerticalVelocity * elapsedTime - self->gravity * elapsedTime * elapsedTime * 0.5;
    if (self->yAvoidance != 0 && normalTime < 0.25) {
        float num3 = 0.5 - std::cos(normalTime * 8 * M_PI) * 0.5;
        self->localPosition.y = self->localPosition.y + num3 * self->yAvoidance;
    }
    if (normalTime < 0.5) {
        Transform *baseTransform = self->get_transform();
        Quaternion a;
        if (normalTime < 0.125) {
            a = Quaternion::Slerp(baseTransform->get_rotation() * self->startRotation, 
                                  baseTransform->get_rotation() * self->middleRotation,
                                  std::sin(normalTime * M_PI * 4));
        } else {
            a = Quaternion::Slerp(baseTransform->get_rotation() * self->middleRotation, 
                                  baseTransform->get_rotation() * self->endRotation,
                                  std::sin((normalTime - 0.125) * M_PI * 2));
        }

        Vector3 vector = self->playerTransforms->headWorldPos;

        // Aero doesn't know what's happening anymore
        Quaternion worldRot = self->inverseWorldRotation;
        if (baseTransform->get_parent()) {
            // Handle parenting
            worldRot = worldRot * Quaternion::Inverse(baseTransform->get_parent()->get_rotation());
        }

        Transform *headTransform = self->playerTransforms->headTransform;
        Quaternion inverse = Quaternion::Inverse(worldRot);
        Vector3 upVector = inverse * Vector3::get_up();
        float baseUpMagnitude = Vector3::Dot(worldRot * baseTransform->get_position(), Vector3::get_up());
        float headUpMagnitude = Vector3::Dot(worldRot * headTransform->get_position(), Vector3::get_up());
        float mult = std::lerp(headUpMagnitude, baseUpMagnitude, 0.8f) - headUpMagnitude;
        vector = vector + upVector * mult;

        // more wtf
        Vector3 normalized = baseTransform->get_rotation() * (worldRot * (baseTransform->get_position() - vector).get_normalized());

        Quaternion b = Quaternion::LookRotation(normalized, self->rotatedObject->get_up());
        self->rotatedObject->set_rotation(Quaternion::Lerp(a, b, normalTime * 2));
    }
    if (normalTime >= 0.5 && !self->halfJumpMarkReported) {
        self->halfJumpMarkReported = true;
        if (self->noteJumpDidPassHalfEvent) self->noteJumpDidPassHalfEvent->Invoke();
    }
    if (normalTime >= 0.75 && !self->threeQuartersMarkReported) {
        self->threeQuartersMarkReported = true;
        if (self->noteJumpDidPassThreeQuartersEvent) self->noteJumpDidPassThreeQuartersEvent->Invoke(self);
    }
    if (songTime >= self->missedTime && !self->missedMarkReported) {
        self->missedMarkReported = true;
        if (self->noteJumpDidPassMissedMarkerEvent) self->noteJumpDidPassMissedMarkerEvent->Invoke();
    }
    if (self->threeQuartersMarkReported) {
        float num4 = (normalTime - 0.75f) / 0.25f;
        num4 = num4 * num4 * num4;
        self->localPosition.z = self->localPosition.z - std::lerp(0, self->endDistanceOffset, num4);
    }
    if (normalTime >= 1) {
        if (self->noteJumpDidFinishEvent) self->noteJumpDidFinishEvent->Invoke();
    }

    Vector3 result = self->worldRotation * self->localPosition;
    self->get_transform()->set_localPosition(self->worldRotation * self->localPosition);
    if (self->noteJumpDidUpdateProgressEvent) {
        self->noteJumpDidUpdateProgressEvent->Invoke(normalTime);
    }

    if (noteUpdateAD) {
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
    INSTALL_HOOK_ORIG(logger, NoteJump_ManualUpdate);
}

NEInstallHooks(InstallNoteJumpHooks);
