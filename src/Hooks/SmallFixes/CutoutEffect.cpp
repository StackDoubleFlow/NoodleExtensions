#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "UnityEngine/Vector3.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

MAKE_HOOK_MATCH(CutoutEffect_SetCutout,
                static_cast<void (GlobalNamespace::CutoutEffect::*)(float, UnityEngine::Vector3)>(&GlobalNamespace::CutoutEffect::SetCutout), void,
                CutoutEffect *self,
                float cutout, UnityEngine::Vector3 cutoutOffset) {
    // Do not run SetCutout if the new value is the same as old.
    if (cutout != self->cutout) {
        CutoutEffect_SetCutout(self, cutout, cutoutOffset);
    }
}

void InstallCutoutEffectHooks(Logger &logger) {
    INSTALL_HOOK(logger, CutoutEffect_SetCutout);
}

NEInstallHooks(InstallCutoutEffectHooks);