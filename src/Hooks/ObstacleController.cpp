#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleDissolve.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"

#include "NEConfig.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

Quaternion GetWorldRotation(float def,
                            CustomJSONData::CustomObstacleData *obstacleData) {
    Quaternion worldRotation = Quaternion::Euler(0, def, 0);
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        if (customData.HasMember("_rotation")) {
            if (customData["_rotation"].IsArray()) {
                float x = customData["_rotation"][0].GetFloat();
                float y = customData["_rotation"][1].GetFloat();
                float z = customData["_rotation"][2].GetFloat();
                worldRotation = Quaternion::Euler(x, y, z);
            } else {
                worldRotation =
                    Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }
        }
    }
    return worldRotation;
}

float GetCustomWidth(float def,
                     CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value *> scale =
            customData.HasMember("_scale")
                ? std::optional{&customData["_scale"]}
                : std::nullopt;
        std::optional<float> width =
            scale.has_value() ? std::optional{(**scale)[0].GetFloat()}
                              : std::nullopt;
        if (width.has_value()) {
            return *width;
        }
    }
    return def;
}

float GetCustomLength(float def,
                      CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value *> scale =
            customData.HasMember("_scale")
                ? std::optional{&customData["_scale"]}
                : std::nullopt;
        if (scale.has_value() && scale.value()->Size() > 2) {
            return (**scale)[2].GetFloat() * /*NoteLinesDistace*/ 0.6;
        }
    }
    return def;
}

MAKE_HOOK_MATCH(ObstacleController_Init, &ObstacleController::Init, void,
                ObstacleController *self, ObstacleData *normalObstacleData,
                float worldRotation, Vector3 startPos, Vector3 midPos,
                Vector3 endPos, float move1Duration, float move2Duration,
                float singleLineWidth, float height) {
    ObstacleController_Init(self, normalObstacleData, worldRotation, startPos,
                            midPos, endPos, move1Duration, move2Duration,
                            singleLineWidth, height);
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(
        normalObstacleData);

    Transform *transform = self->get_transform();
    transform->set_localScale(Vector3::get_one());

    if (!obstacleData->customData->value) {
        return;
    }
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    Quaternion rotation = GetWorldRotation(worldRotation, obstacleData);
    self->worldRotation = rotation;
    self->inverseWorldRotation = Quaternion::Euler(-rotation.get_eulerAngles());

    float width =
        GetCustomWidth(obstacleData->width, obstacleData) * singleLineWidth;
    Vector3 b = Vector3{(width - singleLineWidth) * 0.5f, 0, 0};
    self->startPos = startPos + b;
    self->midPos = midPos + b;
    self->endPos = endPos + b;
    ad.moveStartPos = self->startPos;
    ad.moveEndPos = self->midPos;
    ad.jumpEndPos = self->endPos;

    float defaultLength = (self->endPos - self->midPos).get_magnitude() /
                          move2Duration * obstacleData->duration;
    float length = GetCustomLength(defaultLength, obstacleData);

    rapidjson::Value &customData = *obstacleData->customData->value;

    self->stretchableObstacle->SetSizeAndColor(
        width * 0.98, height, length,
        self->stretchableObstacle->obstacleFrame->color);
    self->bounds = self->stretchableObstacle->bounds;

    std::optional<rapidjson::Value *> localrot =
        customData.HasMember("_localRotation")
            ? std::optional{&customData["_localRotation"]}
            : std::nullopt;

    Quaternion localRotation = Quaternion::get_identity();
    if (localrot.has_value()) {
        localRotation = Quaternion::Euler((**localrot)[0].GetFloat(),
                                          (**localrot)[1].GetFloat(),
                                          (**localrot)[2].GetFloat());
    }
    transform->set_localPosition(startPos);
    transform->set_localRotation(self->worldRotation * localRotation);
    ad.localRotation = localRotation;
    ad.worldRotation = rotation;

    Track *track = ad.track;
    if (track) {
        ParentObject *parentObject =
            ParentController::GetParentObjectTrack(track);
        if (parentObject) {
            parentObject->ParentToObject(transform);
        } else {
            ParentObject::ResetTransformParent(transform);
        }
    } else {
        ParentObject::ResetTransformParent(transform);
    }

    std::optional<bool> cuttable =
        customData.HasMember("_cuttable")
            ? std::optional{customData["_cuttable"].GetBool()}
            : std::nullopt;
    if (cuttable && !*cuttable) {
        self->bounds.set_size(Vector3::get_zero());
    }

    self->Update();
}

MAKE_HOOK_MATCH(ObstacleController_ManualUpdate, &ObstacleController::ManualUpdate, void,
                ObstacleController *self) {
    auto *obstacleData =
        reinterpret_cast<CustomJSONData::CustomObstacleData *>(self->obstacleData);

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

    float songTime = self->audioTimeSyncController->get_songTime();
    float elapsedTime = songTime - self->startTimeOffset;
    float normalTime = (elapsedTime - self->move1Duration) /
                       (self->move2Duration + self->obstacleDuration);

    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(
        ad.animationData, ad.track, normalTime);

    if (offset.positionOffset.has_value()) {
        self->startPos = ad.moveStartPos + *offset.positionOffset;
        self->midPos = ad.moveEndPos + *offset.positionOffset;
        self->endPos = ad.jumpEndPos + *offset.positionOffset;
    }

    Transform *transform = self->get_transform();

    if (offset.scaleOffset.has_value()) {
        transform->set_localScale(*offset.scaleOffset);
    }

    if (offset.rotationOffset.has_value() ||
        offset.localRotationOffset.has_value()) {
        Quaternion worldRotation = ad.worldRotation;
        Quaternion localRotation = ad.localRotation;

        Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion =
                worldRotationQuaternion * *offset.rotationOffset;
            Quaternion inverseWorldRotation =
                Quaternion::Inverse(worldRotationQuaternion);
            self->worldRotation = worldRotationQuaternion;
            self->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion =
                worldRotationQuaternion * *offset.localRotationOffset;
        }

        transform->set_localRotation(worldRotationQuaternion);
    }

    if (offset.cuttable.has_value() && !*offset.cuttable) {
        self->bounds.set_size(Vector3::get_zero());
    }

    if (offset.dissolve.has_value() && getNEConfig().enableObstacleDissolve.GetValue()) {
        ConditionalMaterialSwitcher *materialSwitcher = ad.materialSwitcher;
        if (!materialSwitcher) {
            materialSwitcher = self->get_gameObject()->GetComponentInChildren<ConditionalMaterialSwitcher *>();
            ad.materialSwitcher = materialSwitcher;
        }
        if (!materialSwitcher->value->get_value()) {
            materialSwitcher->value->set_value(true);
        }

        CutoutAnimateEffect *cutoutAnimationEffect = ad.cutoutAnimationEffect;
        if (!cutoutAnimationEffect) {
            ObstacleDissolve *obstacleDissolve =
                self->get_gameObject()->GetComponent<ObstacleDissolve *>();
            cutoutAnimationEffect = obstacleDissolve->cutoutAnimateEffect;
            ad.cutoutAnimationEffect = cutoutAnimationEffect;
        }

        cutoutAnimationEffect->SetCutout(1 - *offset.dissolve);

        // if (offset.dissolve <= 0) {
        //     cutoutAnimationEffect->SetCutout(1);
        // } else {
        //     cutoutAnimationEffect->SetCutout(0);
        // }
    }

    ObstacleController_ManualUpdate(self);
}

MAKE_HOOK_MATCH(ObstacleController_GetPosForTime,
                &ObstacleController::GetPosForTime, Vector3,
                ObstacleController *self, float time) {
    auto *obstacleData =
        (CustomJSONData::CustomObstacleData *)self->obstacleData;

    if (!obstacleData->customData->value) {
        return ObstacleController_GetPosForTime(self, time);
    }
    rapidjson::Value &customData = *obstacleData->customData->value;
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    float jumpTime =
        std::clamp((time - self->move1Duration) /
                       (self->move2Duration + self->obstacleDuration),
                   0.0f, 1.0f);
    std::optional<Vector3> position =
        AnimationHelper::GetDefinitePositionOffset(ad.animationData, ad.track,
                                                   jumpTime);

    if (position.has_value()) {
        Vector3 noteOffset = ad.noteOffset;
        Vector3 definitePosition = *position + noteOffset;
        definitePosition.x += ad.xOffset;
        if (time < self->move1Duration) {
            Vector3 result = Vector3::LerpUnclamped(
                self->startPos, self->midPos, time / self->move1Duration);
            return result + definitePosition - self->midPos;
        } else {
            return definitePosition;
        }
    }

    return ObstacleController_GetPosForTime(self, time);
}

MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_OnEnable,
                &ParametricBoxFakeGlowController::OnEnable, void,
                ParametricBoxFakeGlowController *self) {}

void InstallObstacleControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, ObstacleController_Init);
    INSTALL_HOOK(logger, ObstacleController_ManualUpdate);
    INSTALL_HOOK(logger, ObstacleController_GetPosForTime);
    // Temporary fake glow disable hook
    INSTALL_HOOK(logger, ParametricBoxFakeGlowController_OnEnable);
}

NEInstallHooks(InstallObstacleControllerHooks);