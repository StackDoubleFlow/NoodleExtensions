#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"
#include "NECaches.h"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/EnvironmentSpawnRotation.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(PauseMenuManager_ShowMenu, &PauseMenuManager::ShowMenu, void, PauseMenuManager* self) {
  if (!Hooks::isNoodleHookEnabled()) return PauseMenuManager_ShowMenu(self);

  auto transform = self->_pauseContainerTransform->get_transform();
  auto eulerAngle = transform->get_eulerAngles();
  PauseMenuManager_ShowMenu(self);
  transform->set_eulerAngles(eulerAngle);
  transform->set_localEulerAngles({ 0, self->_environmentSpawnRotation->targetRotation, 0 });
}

void InstallPauseManagerHooks(Logger& logger) {
  INSTALL_HOOK(logger, PauseMenuManager_ShowMenu);
}

NEInstallHooks(InstallPauseManagerHooks);