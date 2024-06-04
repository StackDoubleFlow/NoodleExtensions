#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/EnvironmentSceneSetup.hpp"

#include "UnityEngine/Shader.hpp"

#include "Zenject/DiContainer.hpp"
#include "NECaches.h"
#include "NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(EnvironmentSceneSetup_InstallBindings, &EnvironmentSceneSetup::InstallBindings, void,
                EnvironmentSceneSetup* self) {
  EnvironmentSceneSetup_InstallBindings(self);

  if (!Hooks::isNoodleHookEnabled()) return;

  static int const _trackLaneYPositionPropertyId = UnityEngine::Shader::PropertyToID("_TrackLaneYPosition");

  UnityEngine::Shader::SetGlobalFloat(_trackLaneYPositionPropertyId, -1000000.0f);
}

void InstallEnvironmentSceneSetupHooks() {
  INSTALL_HOOK(NELogger::Logger, EnvironmentSceneSetup_InstallBindings);
}

NEInstallHooks(InstallEnvironmentSceneSetupHooks);