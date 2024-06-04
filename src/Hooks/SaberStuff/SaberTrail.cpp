
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/SaberTrail.hpp"
#include "GlobalNamespace/SaberTrailRenderer.hpp"

#include "UnityEngine/Transform.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <optional>

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(SaberTrail_Init, &SaberTrail::Init, void, SaberTrail* self) {
  if (!Hooks::isNoodleHookEnabled()) return SaberTrail_Init(self);

  SaberTrail_Init(self);
  // parent to VRGameCore

  auto vrGameCore = self->get_transform()->get_parent()->get_parent()->get_parent();

  self->_trailRenderer->get_transform()->SetParent(vrGameCore, false);
}

void InstallSaber_TrailHooks() {
  INSTALL_HOOK(NELogger::Logger, SaberTrail_Init);
}

NEInstallHooks(InstallSaber_TrailHooks);