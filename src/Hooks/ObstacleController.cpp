#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleDissolve.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEConfig.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/AssociatedData.h"
#include "tracks/shared/TimeSourceHelper.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

std::unordered_map<ObstacleController *, Array<ConditionalMaterialSwitcher *> *> cachedObstacleMaterialSwitchers;

void NECaches::ClearObstacleCaches() {
    cachedObstacleMaterialSwitchers.clear();
}

MAKE_HOOK_MATCH(ObstacleController_Init, &ObstacleController::Init, void, ObstacleController *self,
                ObstacleData *normalObstacleData, float worldRotation, Vector3 startPos,
                Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration,
                float singleLineWidth, float height) {
    ObstacleController_Init(self, normalObstacleData, worldRotation, startPos, midPos, endPos,
                            move1Duration, move2Duration, singleLineWidth, height);
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(normalObstacleData);

    Transform *transform = self->get_transform();
    transform->set_localScale(NEVector::Vector3::get_one());

    if (!obstacleData->customData->value) {
        return;
    }
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    Quaternion rotation =
        Quaternion::Euler(ad.objectData.rotation.value_or(Vector3(0, worldRotation, 0)));
    self->worldRotation = rotation;
    self->inverseWorldRotation = NEVector::Quaternion::Euler(-rotation.get_eulerAngles());

    auto &scale = ad.objectData.scale;

    float width = (scale && scale->at(0) ? *scale->at(0) : obstacleData->width) * singleLineWidth;
    NEVector::Vector3 b = NEVector::Vector3((width - singleLineWidth) * 0.5f, 0, 0);
    self->startPos = startPos + b;
    self->midPos = midPos + b;
    self->endPos = endPos + b;
    ad.moveStartPos = self->startPos;
    ad.moveEndPos = self->midPos;
    ad.jumpEndPos = self->endPos;

    float defaultLength =
        (self->endPos - self->midPos).get_magnitude() / move2Duration * obstacleData->duration;
    float length = (scale && scale->at(2) ? *scale->at(2) : defaultLength);

    self->stretchableObstacle->SetSizeAndColor(width * 0.98, height, length,
                                               self->stretchableObstacle->obstacleFrame->color);
    self->bounds = self->stretchableObstacle->bounds;

    std::optional<NEVector::Vector3> &localrot = ad.objectData.localRotation;

    Quaternion localRotation = NEVector::Quaternion::get_identity();
    if (localrot.has_value()) {
        localRotation = NEVector::Quaternion::Euler(localrot->x, localrot->y, localrot->z);
    }
    transform->set_localPosition(startPos);
    transform->set_localRotation(self->worldRotation * localRotation);
    ad.localRotation = localRotation;
    ad.worldRotation = rotation;

    Track *track = TracksAD::getAD(obstacleData->customData).track;
    if (track) {
        ParentObject *parentObject = ParentController::GetParentObjectTrack(track);
        if (parentObject) {
            parentObject->ParentToObject(transform);
        } else {
            ParentObject::ResetTransformParent(transform);
        }
    } else {
        ParentObject::ResetTransformParent(transform);
    }

    std::optional<bool> &cuttable = ad.objectData.interactable;
    if (cuttable && !*cuttable) {
        self->bounds.set_size(Vector3::get_zero());
    }

    if (getNEConfig().enableObstacleDissolve.GetValue()) {
        Array<ConditionalMaterialSwitcher *>* materialSwitchers;
        auto it = cachedObstacleMaterialSwitchers.find(self);
        if (it == cachedObstacleMaterialSwitchers.end()) {
            cachedObstacleMaterialSwitchers[self] = materialSwitchers = self->get_gameObject()->GetComponentsInChildren<ConditionalMaterialSwitcher *>();
        } else {
            materialSwitchers = it->second;
        }
        ad.materialSwitchers = materialSwitchers;

        for (auto *materialSwitcher : materialSwitchers->ref_to()) {
            if (materialSwitcher->renderer->get_sharedMaterial() != materialSwitcher->material0) {
                materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material0);
            }
        }
        ad.dissolveEnabled = false;
    }

    self->Update();
}

bool test = false;
MAKE_HOOK_MATCH(ObstacleController_ManualUpdate, &ObstacleController::ManualUpdate, void,
                ObstacleController *self) {
    test = true;
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(self->obstacleData);

    if (!obstacleData->customData->value) {
        ObstacleController_ManualUpdate(self);
        return;
    }
    rapidjson::Value &customData = *obstacleData->customData->value;

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     ObstacleController_Update(self);
    //     return;
    // }

    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);
    Track *track = TracksAD::getAD(obstacleData->customData).track;

    float songTime = TimeSourceHelper::getSongTime(self->audioTimeSyncController);
    float elapsedTime = songTime - self->startTimeOffset;
    float normalTime =
        (elapsedTime - self->move1Duration) / (self->move2Duration + self->obstacleDuration);

    AnimationHelper::ObjectOffset offset =
        AnimationHelper::GetObjectOffset(ad.animationData, track, normalTime);

    if (offset.positionOffset.has_value()) {
        self->startPos = ad.moveStartPos + *offset.positionOffset;
        self->midPos = ad.moveEndPos + *offset.positionOffset;
        self->endPos = ad.jumpEndPos + *offset.positionOffset;
    }

    Transform *transform = self->get_transform();

    if (offset.scaleOffset.has_value()) {
        transform->set_localScale(*offset.scaleOffset);
    }

    if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
        NEVector::Quaternion worldRotation = ad.worldRotation;
        NEVector::Quaternion localRotation = ad.localRotation;

        NEVector::Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
            NEVector::Quaternion inverseWorldRotation =
                NEVector::Quaternion::Inverse(worldRotationQuaternion);
            self->worldRotation = worldRotationQuaternion;
            self->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
        }

        transform->set_localRotation(worldRotationQuaternion);
    }

    if (offset.cuttable.has_value() && !*offset.cuttable) {
        self->bounds.set_size(Vector3::get_zero());
    }

    bool obstacleDissolveConfig = getNEConfig().enableObstacleDissolve.GetValue();
    if (offset.dissolve.has_value() && obstacleDissolveConfig) {
        if (!ad.dissolveEnabled) {
            ArrayWrapper<ConditionalMaterialSwitcher *> materialSwitchers = ad.materialSwitchers;
            for (auto *materialSwitcher : materialSwitchers) {
                materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material1);
            }
            ad.dissolveEnabled = true;
        }

        CutoutAnimateEffect *cutoutAnimationEffect = ad.cutoutAnimationEffect;
        if (!cutoutAnimationEffect) {
            ObstacleDissolve *obstacleDissolve =
                self->get_gameObject()->GetComponent<ObstacleDissolve *>();
            cutoutAnimationEffect = obstacleDissolve->cutoutAnimateEffect;
            ad.cutoutAnimationEffect = cutoutAnimationEffect;
        }

        cutoutAnimationEffect->SetCutout(1 - *offset.dissolve);
    }

    ObstacleController_ManualUpdate(self);
    test = false;
}

MAKE_HOOK_MATCH(ObstacleController_GetPosForTime, &ObstacleController::GetPosForTime, Vector3,
                ObstacleController *self, float time) {
    auto *obstacleData = (CustomJSONData::CustomObstacleData *)self->obstacleData;

    if (!obstacleData->customData->value) {
        return ObstacleController_GetPosForTime(self, time);
    }
    rapidjson::Value &customData = *obstacleData->customData->value;
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);
    Track *track = TracksAD::getAD(obstacleData->customData).track;

    float jumpTime = (time - self->move1Duration) / (self->move2Duration + self->obstacleDuration);
    jumpTime = std::clamp(jumpTime, 0.0f, 1.0f);
    std::optional<NEVector::Vector3> position =
        AnimationHelper::GetDefinitePositionOffset(ad.animationData, track, jumpTime);

    if (position.has_value()) {
        NEVector::Vector3 noteOffset = ad.noteOffset;
        NEVector::Vector3 definitePosition = *position + noteOffset;
        definitePosition.x += ad.xOffset;
        if (time < self->move1Duration) {
            NEVector::Vector3 result = NEVector::Vector3::LerpUnclamped(
                self->startPos, self->midPos, time / self->move1Duration);
            return result + definitePosition - static_cast<NEVector::Vector3>(self->midPos);
        } else {
            return definitePosition;
        }
    }

    return ObstacleController_GetPosForTime(self, time);
}

MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_OnEnable,
                &ParametricBoxFakeGlowController::OnEnable, void,
                ParametricBoxFakeGlowController *self) {}

// #include "beatsaber-hook/shared/utils/instruction-parsing.hpp"
// MAKE_HOOK(Object_New, nullptr, Il2CppObject *, Il2CppClass *klass) {
//     if (test && klass && klass->name && klass->namespaze) {
//         NELogger::GetLogger().info("Allocating a %s.%s", klass->namespaze, klass->name);
//         PrintBacktrace(10);
//     }
//     return Object_New(klass);
// }

void InstallObstacleControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, ObstacleController_Init);
    INSTALL_HOOK(logger, ObstacleController_ManualUpdate);
    INSTALL_HOOK(logger, ObstacleController_GetPosForTime);
    // Temporary fake glow disable hook
    INSTALL_HOOK(logger, ParametricBoxFakeGlowController_OnEnable);

    // Instruction on((const int32_t*) HookTracker::GetOrig(il2cpp_functions::object_new));
    // Instruction j2Ob_N_thunk(CRASH_UNLESS(on.findNthCall(1)->label));
    // logger.info("thunk addr %p %x", j2Ob_N_thunk.addr, *j2Ob_N_thunk.addr);
    // auto *j2Ob_N = CRASH_UNLESS(j2Ob_N_thunk.findNthDirectBranchWithoutLink(1));
    // INSTALL_HOOK_DIRECT(logger, Object_New, (void*) *j2Ob_N->label);
}

NEInstallHooks(InstallObstacleControllerHooks);