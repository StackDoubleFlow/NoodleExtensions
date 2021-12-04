
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/Saber.hpp"

#include "UnityEngine/Transform.hpp"

#include "NEHooks.h"
#include "NELogger.h"

#include <optional>

using namespace GlobalNamespace;

SaberMovementData* movementData = nullptr;
Saber* saber = nullptr;

// We store all positions as localpositions so that abrupt changes in world position do not affect this
// it gets converted back to world position to calculate cut
MAKE_HOOK_MATCH(Saber_ManualUpdate,
                &Saber::ManualUpdate,
                void,
                Saber* self) {
    movementData = self->movementData;
    saber = self;

    Saber_ManualUpdate(self);

    movementData = nullptr;
    saber = nullptr;
}

MAKE_HOOK_MATCH(SaberMovementData_AddNewData,
                &SaberMovementData::AddNewData,
                void,
                SaberMovementData* self,
                UnityEngine::Vector3 topPos, UnityEngine::Vector3 bottomPos, float time) {
    if (self != movementData || !movementData || !saber)
        return SaberMovementData_AddNewData(self, topPos, bottomPos, time);


    auto playerTransform = saber->get_transform()->get_parent()->get_parent();

    topPos = playerTransform->InverseTransformPoint(topPos);
    bottomPos = playerTransform->InverseTransformPoint(bottomPos);

    return SaberMovementData_AddNewData(self, topPos, bottomPos, time);
}

void InstallSaberHooks(Logger &logger) {
    INSTALL_HOOK(logger, Saber_ManualUpdate);
}

NEInstallHooks(InstallSaberHooks);