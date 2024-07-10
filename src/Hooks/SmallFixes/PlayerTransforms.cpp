#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "NEHooks.h"

#include "tracks/shared/Vector.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(PlayerTransforms_Awake, &PlayerTransforms::Awake, void, PlayerTransforms* self) {
  if (!Hooks::isNoodleHookEnabled()) return PlayerTransforms_Awake(self);

  PlayerTransforms_Awake(self);
  self->_useOriginParentTransformForPseudoLocalCalculations = false;
}
MAKE_HOOK_MATCH(PlayerTransforms_HeadsetOffsetZ, &PlayerTransforms::HeadOffsetZ, float, PlayerTransforms* self,
                ::UnityEngine::Quaternion noteInverseWorldRotation) {
  if (!Hooks::isNoodleHookEnabled()) return PlayerTransforms_HeadsetOffsetZ(self, noteInverseWorldRotation);

  // get magnitude in direction we care about rather than just z
  return NEVector::Vector3::Dot(NEVector::Quaternion(noteInverseWorldRotation) * self->_headPseudoLocalPos, self->_originParentTransform->forward);
}

void InstallPlayerTransformsHooks() {
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_Awake);
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_HeadsetOffsetZ);
}

NEInstallHooks(InstallPlayerTransformsHooks);