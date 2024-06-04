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

extern BeatmapObjectAssociatedData* noteUpdateAD;
extern TracksAD::TracksVector noteTracks;

float noteTimeAdjust(float original, float jumpDuration);

constexpr static float InOutQuad(float t) {
  if (t >= 0.5f) {
    return -1.0f + (4.0f - 2.0f * t) * t;
  }
  return 2.0f * t * t;
}

constexpr float NoteMissedTimeAdjust(float beatTime, float jumpDuration, float num) {
  return num + (beatTime - (jumpDuration * 0.5f));
}

void NoteJump_ManualUpdateNoteLookTranspile(NoteJump* self, Transform* selfTransform, float const normalTime) {
  if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
    self->_rotatedObject->set_localRotation(self->_endRotation);
    return;
  }
  Transform* baseTransform = selfTransform; // lazy
  NEVector::Vector3 baseTransformPosition(baseTransform->get_position());
  NEVector::Quaternion baseTransformRotation(baseTransform->get_rotation());

  NEVector::Quaternion a =
      normalTime < 0.125
          ? NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->_startRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->_middleRotation),
                                        std::sin(normalTime * M_PI * 4))
          : NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->_middleRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->_endRotation),
                                        std::sin((normalTime - 0.125) * M_PI * 2));

  NEVector::Vector3 vector = self->_playerTransforms->headWorldPos;

  // Aero doesn't know what's happening anymore
  NEVector::Quaternion worldRot = self->_inverseWorldRotation;
  auto baseTransformParent = baseTransform->get_parent();
  if (baseTransformParent) {
    // Handle parenting
    worldRot = worldRot * (NEVector::Quaternion)NEVector::Quaternion::Inverse(baseTransformParent->get_rotation());
  }

  Transform* headTransform = self->_playerTransforms->_headTransform;
  NEVector::Quaternion inverse = NEVector::Quaternion::Inverse(worldRot);
  NEVector::Vector3 upVector = inverse * NEVector::Vector3::up();

  float baseUpMagnitude = NEVector::Vector3::Dot(worldRot * baseTransformPosition, NEVector::Vector3::up());
  float headUpMagnitude =
      NEVector::Vector3::Dot(worldRot * NEVector::Vector3(headTransform->get_position()), NEVector::Vector3::up());
  float mult = std::lerp(headUpMagnitude, baseUpMagnitude, 0.8f) - headUpMagnitude;
  vector = vector + (upVector * mult);

  // more wtf
  NEVector::Vector3 normalized = NEVector::Quaternion(baseTransformRotation) *
                                 (worldRot * Sombrero::vector3subtract(baseTransformPosition, vector).get_normalized());

  NEVector::Quaternion b = NEVector::Quaternion::LookRotation(normalized, self->_rotatedObject->get_up());
  self->_rotatedObject->set_rotation(NEVector::Quaternion::Lerp(a, b, normalTime * 2));
}

MAKE_HOOK_MATCH(NoteJump_ManualUpdate, &NoteJump::ManualUpdate, Vector3, NoteJump* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteJump_ManualUpdate(self);
  auto selfTransform = self->get_transform();
  float songTime = TimeSourceHelper::getSongTime(self->_audioTimeSyncController);
  float elapsedTime = songTime - (self->_beatTime - self->jumpDuration * 0.5f);
  // transpile here
  if (noteUpdateAD) {
    elapsedTime = noteTimeAdjust(elapsedTime, self->jumpDuration);
  }
  //
  float normalTime = elapsedTime / self->jumpDuration;

  if (self->_startPos.x == self->_endPos.x) {
    self->_localPosition.x = self->_startPos.x;
  } else if (normalTime < 0.25) {
    self->_localPosition.x = self->_startPos.x + (self->_endPos.x - self->_startPos.x) * InOutQuad(normalTime * 4);
  } else {
    self->_localPosition.x = self->_endPos.x;
  }
  self->_localPosition.z =
      self->_playerTransforms->MoveTowardsHead(self->_startPos.z, self->_endPos.z, self->_inverseWorldRotation, normalTime);
  self->_localPosition.y =
      self->_startPos.y + self->_startVerticalVelocity * elapsedTime - self->_gravity * elapsedTime * elapsedTime * 0.5;
  if (self->_yAvoidance != 0 && normalTime < 0.25) {
    float num3 = 0.5 - std::cos(normalTime * 8 * M_PI) * 0.5;
    self->_localPosition.y = self->localPosition.y + num3 * self->_yAvoidance;
  }

  // transpile here
  // https://github.com/Aeroluna/NoodleExtensions/blob/2147129bfd480a718d99d8c2ca8c45df0502c5d1/NoodleExtensions/HarmonyPatches/NoteJump.cs#L115-L126
  bool definitePosition = false;

  if (noteUpdateAD) {
    std::optional<NEVector::Vector3> position =
        AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTracks, normalTime);
    if (position.has_value()) {
      self->_localPosition = *position + noteUpdateAD->noteOffset;
      definitePosition = true;
    }
  }
  //

  if (normalTime < 0.5) {
    NoteJump_ManualUpdateNoteLookTranspile(self, selfTransform, normalTime);
  }
  if (normalTime >= 0.5 && !self->_halfJumpMarkReported) {
    self->_halfJumpMarkReported = true;
    if (self->noteJumpDidPassHalfEvent) self->noteJumpDidPassHalfEvent->Invoke();
  }
  if (normalTime >= 0.75 && !self->_threeQuartersMarkReported) {
    self->_threeQuartersMarkReported = true;
    if (self->noteJumpDidPassThreeQuartersEvent) self->noteJumpDidPassThreeQuartersEvent->Invoke(self);
  }
  if (NoteMissedTimeAdjust(self->_beatTime, self->jumpDuration, elapsedTime) >= self->_missedTime &&
      !self->_missedMarkReported) {
    self->_missedMarkReported = true;
    if (self->noteJumpDidPassMissedMarkerEvent) self->noteJumpDidPassMissedMarkerEvent->Invoke();
  }

  // transpile here
  if (self->_threeQuartersMarkReported && !definitePosition) {
    //
    float num4 = (normalTime - 0.75f) / 0.25f;
    num4 = num4 * num4 * num4;
    self->_localPosition.z = self->localPosition.z - std::lerp(0, self->_endDistanceOffset, num4);
  }
  if (normalTime >= 1) {
    if (self->noteJumpDidFinishEvent) self->noteJumpDidFinishEvent->Invoke();
  }
  if (normalTime >= 1.0f) {
    if (!self->_missedMarkReported) {
      self->_missedMarkReported = true;
      auto action4 = self->noteJumpDidPassMissedMarkerEvent;
      if (action4 != nullptr) {
        action4->Invoke();
      }
    }
    auto action5 = self->noteJumpDidFinishEvent;
    if (action5 != nullptr) {
      action5->Invoke();
    }
  }

  NEVector::Vector3 result = NEVector::Quaternion(self->_worldRotation) * NEVector::Vector3(self->localPosition);
  selfTransform->set_localPosition(result);
  if (self->noteJumpDidUpdateProgressEvent) {
    self->noteJumpDidUpdateProgressEvent->Invoke(normalTime);
  }

  return result;
}

void InstallNoteJumpHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, NoteJump_ManualUpdate);
}

NEInstallHooks(InstallNoteJumpHooks);
