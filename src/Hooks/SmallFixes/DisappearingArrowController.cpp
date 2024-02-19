#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

template <>
struct ::il2cpp_utils::il2cpp_type_check::MetadataGetter<
    &DisappearingArrowControllerBase_1<GameNoteController*>::SetArrowTransparency> {
  static MethodInfo const* methodInfo() {
    return il2cpp_utils::FindMethodUnsafe(
        classof(GlobalNamespace::DisappearingArrowControllerBase_1<GameNoteController*>*), "SetArrowTransparency", 1);
  }
};

MAKE_HOOK_MATCH(DisappearingArrowControllerBase_SetArrowTransparency,
                &DisappearingArrowControllerBase_1<GameNoteController*>::SetArrowTransparency, void,
                DisappearingArrowControllerBase_1<GameNoteController*>* self, float arrowTransparency) {
  DisappearingArrowControllerBase_SetArrowTransparency(self, arrowTransparency);
  if (!Hooks::isNoodleHookEnabled()) return;
  if (!self->_arrowCutoutEffect) return;
  self->_arrowCutoutEffect->SetCutout(1.0f - arrowTransparency);
}

void InstallDisappearingArrowControllerHooks(Logger& logger) {
  INSTALL_HOOK(logger, DisappearingArrowControllerBase_SetArrowTransparency);
}

NEInstallHooks(InstallDisappearingArrowControllerHooks);