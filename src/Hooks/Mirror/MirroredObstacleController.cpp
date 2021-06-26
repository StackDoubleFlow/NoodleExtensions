#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/MirroredObstacleController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "UnityEngine/Transform.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;


MAKE_HOOK_OFFSETLESS(MirroredObstacleController_UpdatePositionAndRotation, void, MirroredObstacleController *self) {
    if (self->followedTransform->get_position().y < 0) {
        // Hide without disabling update
        self->transform->set_position(Vector3(0, 100, 0));
    }

    Vector3 position = self->followedTransform->get_position();
    Quaternion quaternion = self->followedTransform->get_rotation();
    position.y = -position.y;

    // TODO:
    // quaternion = quaternion.Reflect(Vector3::get_up());

    self->transform->SetPositionAndRotation(position, quaternion);

    if (self->followedTransform->get_localScale() != self->transform->get_localScale()) {
        self->transform->set_localScale(self->followedTransform->get_localScale());
    }
}

void InstallMirroredObstacleControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, MirroredObstacleController_UpdatePositionAndRotation, il2cpp_utils::FindMethodUnsafe("", "MirroredObstacleController", "UpdatePositionAndRotation", 0));
}

NEInstallHooks(InstallMirroredObstacleControllerHooks);