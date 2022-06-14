#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/Easing.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "System/Action.hpp"
#include "System/Action_1.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "NEHooks.h"
#include "tracks/shared/TimeSourceHelper.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData *noteUpdateAD;
extern TracksAD::TracksVector noteTracks;

float noteTimeAdjust(float original, float jumpDuration);

constexpr static float InOutQuad(float t)
{
    if (t >= 0.5f)
    {
        return -1.0f + (4.0f - 2.0f * t) * t;
    }
    return 2.0f * t * t;
}


void NoteJump_ManualUpdateNoteLookTranspile(NoteJump *self, Transform* selfTransform, float const normalTime) {
    if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
        self->rotatedObject->set_localRotation(self->endRotation);
        return;
    }
    Transform *baseTransform = selfTransform; // lazy
    NEVector::Vector3 baseTransformPosition(baseTransform->get_position());
    NEVector::Quaternion baseTransformRotation(baseTransform->get_rotation());

    NEVector::Quaternion a;
    if (normalTime < 0.125) {
        a = NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->startRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->middleRotation),
                                        std::sin(normalTime * M_PI * 4));
    } else {
        a = NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->middleRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->endRotation),
                                        std::sin((normalTime - 0.125) * M_PI * 2));
    }

    NEVector::Vector3 vector = self->playerTransforms->headWorldPos;

    // Aero doesn't know what's happening anymore
    NEVector::Quaternion worldRot = self->inverseWorldRotation;
    auto baseTransformParent = baseTransform->get_parent();
    if (baseTransformParent) {
        // Handle parenting
        worldRot = worldRot * (NEVector::Quaternion) NEVector::Quaternion::Inverse(baseTransformParent->get_rotation());
    }

    Transform *headTransform = self->playerTransforms->headTransform;
    NEVector::Quaternion inverse = NEVector::Quaternion::Inverse(worldRot);
    NEVector::Vector3 upVector = inverse * NEVector::Vector3::up();


    float baseUpMagnitude =
            NEVector::Vector3::Dot(worldRot * baseTransformPosition, NEVector::Vector3::up());
    float headUpMagnitude =
            NEVector::Vector3::Dot(worldRot * NEVector::Vector3(headTransform->get_position()), NEVector::Vector3::up());
    float mult = std::lerp(headUpMagnitude, baseUpMagnitude, 0.8f) - headUpMagnitude;
    vector = vector + (upVector * mult);

    // more wtf
    NEVector::Vector3 normalized = NEVector::Quaternion(baseTransformRotation) *
                                   (worldRot * Sombrero::vector3subtract(baseTransformPosition, vector).get_normalized());

    NEVector::Quaternion b = NEVector::Quaternion::LookRotation(normalized, self->rotatedObject->get_up());
    self->rotatedObject->set_rotation(NEVector::Quaternion::Lerp(a, b, normalTime * 2));
}

MAKE_HOOK_MATCH(NoteJump_ManualUpdate, &NoteJump::ManualUpdate, Vector3, NoteJump *self) {
    if (!Hooks::isNoodleHookEnabled())
        return NoteJump_ManualUpdate(self);
    auto selfTransform = self->get_transform();
    float songTime = TimeSourceHelper::getSongTime(self->audioTimeSyncController);
    float elapsedTime = songTime - (self->beatTime - self->jumpDuration * 0.5f);
    // transpile here
    if (noteUpdateAD) {
        elapsedTime = noteTimeAdjust(elapsedTime, self->jumpDuration);
    }
    //
    float normalTime = elapsedTime / self->jumpDuration;

    if (self->startPos.x == self->endPos.x) {
        self->localPosition.x = self->startPos.x;
    } else if (normalTime < 0.25) {
        self->localPosition.x = self->startPos.x + (self->endPos.x - self->startPos.x) *
                                                       InOutQuad(normalTime * 4);
    } else {
        self->localPosition.x = self->endPos.x;
    }
    self->localPosition.z = self->playerTransforms->MoveTowardsHead(
        self->startPos.z, self->endPos.z, self->inverseWorldRotation, normalTime);
    self->localPosition.y = self->startPos.y + self->startVerticalVelocity * elapsedTime -
                            self->gravity * elapsedTime * elapsedTime * 0.5;
    if (self->yAvoidance != 0 && normalTime < 0.25) {
        float num3 = 0.5 - std::cos(normalTime * 8 * M_PI) * 0.5;
        self->localPosition.y = self->localPosition.y + num3 * self->yAvoidance;
    }

    // transpile here
    // https://github.com/Aeroluna/NoodleExtensions/blob/2147129bfd480a718d99d8c2ca8c45df0502c5d1/NoodleExtensions/HarmonyPatches/NoteJump.cs#L115-L126
    bool definitePosition = false;

    if (noteUpdateAD) {
        std::optional<NEVector::Vector3> position = AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTracks, normalTime);
        if (position.has_value()) {
            self->localPosition = *position + noteUpdateAD->noteOffset;
            definitePosition = true;
        }
    }
    //

    if (normalTime < 0.5) {
        NoteJump_ManualUpdateNoteLookTranspile(self, selfTransform, normalTime);
    }
    if (normalTime >= 0.5 && !self->halfJumpMarkReported) {
        self->halfJumpMarkReported = true;
        if (self->noteJumpDidPassHalfEvent)
            self->noteJumpDidPassHalfEvent->Invoke();
    }
    if (normalTime >= 0.75 && !self->threeQuartersMarkReported) {
        self->threeQuartersMarkReported = true;
        if (self->noteJumpDidPassThreeQuartersEvent)
            self->noteJumpDidPassThreeQuartersEvent->Invoke(self);
    }
    if (songTime >= self->missedTime && !self->missedMarkReported) {
        self->missedMarkReported = true;
        if (self->noteJumpDidPassMissedMarkerEvent)
            self->noteJumpDidPassMissedMarkerEvent->Invoke();
    }

    // transpile here
    if (self->threeQuartersMarkReported && !definitePosition) {
    //
        float num4 = (normalTime - 0.75f) / 0.25f;
        num4 = num4 * num4 * num4;
        self->localPosition.z = self->localPosition.z - std::lerp(0, self->endDistanceOffset, num4);
    }
    if (normalTime >= 1) {
        if (self->noteJumpDidFinishEvent)
            self->noteJumpDidFinishEvent->Invoke();
    }
    if (normalTime >= 1.0f)
    {
        if (!self->missedMarkReported)
        {
            self->missedMarkReported = true;
            auto action4 = self->noteJumpDidPassMissedMarkerEvent;
            if (action4 != nullptr)
            {
                action4->Invoke();
            }
        }
        auto action5 = self->noteJumpDidFinishEvent;
        if (action5 != nullptr)
        {
            action5->Invoke();
        }
    }

    NEVector::Vector3 result = NEVector::Quaternion(self->worldRotation) * NEVector::Vector3(self->localPosition);
    selfTransform->set_localPosition(result);
    if (self->noteJumpDidUpdateProgressEvent) {
        self->noteJumpDidUpdateProgressEvent->Invoke(normalTime);
    }

    return result;
}

void InstallNoteJumpHooks(Logger &logger) { INSTALL_HOOK_ORIG(logger, NoteJump_ManualUpdate); }

NEInstallHooks(InstallNoteJumpHooks);
