#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(PlayerTransforms_Update, &PlayerTransforms::Update, void, PlayerTransforms* self) {
  if (!Hooks::isNoodleHookEnabled()) return PlayerTransforms_Update(self);

  PlayerTransforms_Update(self);
  self->headPseudoLocalPos = self->headTransform->get_localPosition();
}

void InstallPlayerTransformsHooks(Logger& logger) {
  INSTALL_HOOK(logger, PlayerTransforms_Update);
}

NEInstallHooks(InstallPlayerTransformsHooks);