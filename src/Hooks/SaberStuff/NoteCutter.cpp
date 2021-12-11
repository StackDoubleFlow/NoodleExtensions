
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

#include "GlobalNamespace/NoteCutter.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParams.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParamsComparer.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/BladeMovementDataElement.hpp"
#include "GlobalNamespace/GeometryTools.hpp"
#include "GlobalNamespace/LayerMasks.hpp"

#include "UnityEngine/Physics.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Collider.hpp"

#include "System/Array.hpp"

#include "NEHooks.h"
#include "NELogger.h"
#include "GlobalNamespace/CuttableBySaber.hpp"
#include "tracks/shared/Vector.h"

#include <optional>

using namespace GlobalNamespace;
using namespace UnityEngine;

UnityEngine::Transform* playerTransform;
std::array<bool, 2> firstCall;

MAKE_HOOK_MATCH(Saber_get_saberBladeTopPos,
                &Saber::get_saberBladeTopPos,
                UnityEngine::Vector3,
                Saber* self) {
    if (!Hooks::isNoodleHookEnabled())
        return Saber_get_saberBladeTopPos(self);

    if (!playerTransform || !firstCall[0])
        return Saber_get_saberBladeTopPos(self);

    auto result = Saber_get_saberBladeTopPos(self);

    firstCall[0] = false;

    return playerTransform->TransformPoint(result);
}

MAKE_HOOK_MATCH(Saber_get_saberBladeBottomPos,
                &Saber::get_saberBladeBottomPos,
                UnityEngine::Vector3,
                Saber* self) {
    if (!Hooks::isNoodleHookEnabled())
        return Saber_get_saberBladeBottomPos(self);

    if (!playerTransform || !firstCall[1])
        return Saber_get_saberBladeBottomPos(self);

    auto result = Saber_get_saberBladeBottomPos(self);

    firstCall[1] = false;

    return playerTransform->TransformPoint(result);
}

MAKE_HOOK_MATCH(NoteCutter_Cut,
                &NoteCutter::Cut,
                void,
                NoteCutter* self,
                Saber* saber) {
    if (!Hooks::isNoodleHookEnabled())
        return NoteCutter_Cut(self, saber);

    firstCall[0] = true;
    firstCall[1] = true;

    playerTransform = saber->get_transform()->get_parent()->get_parent();

    NoteCutter_Cut(self, saber);
    firstCall[0] = false;
    firstCall[1] = false;
    playerTransform = nullptr;
}

void InstallNoteCutterHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutter_Cut);
    INSTALL_HOOK(logger, Saber_get_saberBladeBottomPos);
    INSTALL_HOOK(logger, Saber_get_saberBladeTopPos);
}

NEInstallHooks(InstallNoteCutterHooks);