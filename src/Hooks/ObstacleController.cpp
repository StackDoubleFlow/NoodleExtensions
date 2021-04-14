#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Transform.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/ObstacleDissolve.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

Quaternion GetWorldRotation(float def, CustomJSONData::CustomObstacleData *obstacleData) {
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
                worldRotation = Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }
        }
    }
    return worldRotation;
}

float GetCustomWidth(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
        std::optional<float> width = scale.has_value() ? std::optional{(*scale.value())[0].GetFloat()} : std::nullopt;
        if (width.has_value()) {
            return width.value();
        }
    }
    return def;
}

float GetCustomLength(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
        if (scale.has_value() && scale.value()->Size() > 2) {
            return (*scale.value())[2].GetFloat() * /*NoteLinesDistace*/ 0.6;
        }
    }
    return def;
}

MAKE_HOOK_OFFSETLESS(ObstacleController_Init, void, ObstacleController *self, CustomJSONData::CustomObstacleData *obstacleData, float worldRotation, Vector3 startPos, Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration, float singleLineWidth, float height) {
    ObstacleController_Init(self, obstacleData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, singleLineWidth, height);

    transform->set_localScale(Vector3::get_one());

    if (!obstacleData->customData->value) {
        return;
    }
    BeatmapObjectAssociatedData *ad = getAD(obstacleData->customData);

    Quaternion rotation = GetWorldRotation(worldRotation, obstacleData);
    self->worldRotation = rotation;
    self->inverseWorldRotation = Quaternion::Euler(-rotation.get_eulerAngles());

    float width = GetCustomWidth(obstacleData->width, obstacleData) * singleLineWidth;
    Vector3 b = Vector3 { (width - singleLineWidth) * 0.5f, 0, 0 };
    self->startPos = startPos + b;
    self->midPos = midPos + b;
    self->endPos = endPos + b;
    ad->moveStartPos = self->startPos;
    ad->moveEndPos = self->midPos;
    ad->jumpEndPos = self->endPos;

    float defaultLength = (self->endPos - self->midPos).get_magnitude() / move2Duration * obstacleData->duration;
    float length = GetCustomLength(defaultLength, obstacleData);

    rapidjson::Value &customData = *obstacleData->customData->value;

    self->stretchableObstacle->SetSizeAndColor(width * 0.98, height, length, self->stretchableObstacle->obstacleFrame->color);
    self->bounds = self->stretchableObstacle->bounds;

    std::optional<rapidjson::Value*> localrot = customData.HasMember("_localRotation") ? std::optional{&customData["_localRotation"]} : std::nullopt;

    Transform *transform = self->get_transform();

    Quaternion localRotation = Quaternion::get_identity();
    if (localrot.has_value()) {
        localRotation = Quaternion::Euler((*localrot.value())[0].GetFloat(), (*localrot.value())[1].GetFloat(), (*localrot.value())[2].GetFloat());
    }
    transform->set_localPosition(startPos);
    transform->set_localRotation(self->worldRotation * localRotation);
    ad->localRotation = localRotation;
    ad->worldRotation = rotation;

    Track *track = ad->track;
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

    self->Update();
}

MAKE_HOOK_OFFSETLESS(ObstacleController_Update, void, ObstacleController *self) {
    auto *obstacleData = (CustomJSONData::CustomObstacleData*) self->obstacleData;

    if (!obstacleData->customData->value) {
        ObstacleController_Update(self);
        return;
    }
    rapidjson::Value &customData = *obstacleData->customData->value;

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     ObstacleController_Update(self);
    //     return;
    // }

    BeatmapObjectAssociatedData *ad = getAD(obstacleData->customData);

    float songTime = self->audioTimeSyncController->get_songTime();
    float elapsedTime = songTime - self->startTimeOffset;
    float normalTime = (elapsedTime - self->move1Duration) / (self->move2Duration + self->obstacleDuration);
    
    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(ad->animationData, ad->track, normalTime);

    if (offset.positionOffset.has_value()) {
        self->startPos = ad->moveStartPos + *offset.positionOffset;
        self->midPos = ad->moveEndPos + *offset.positionOffset;
        self->endPos = ad->jumpEndPos + *offset.positionOffset;
    }

    if (offset.scaleOffset.has_value()) {
        self->get_transform()->set_localScale(*offset.scaleOffset);
    }

    if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
        Quaternion worldRotation = ad->worldRotation;
        Quaternion localRotation = ad->localRotation;

        Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
            Quaternion inverseWorldRotation = Quaternion::Euler(-worldRotationQuaternion.get_eulerAngles());
            self->worldRotation = worldRotationQuaternion;
            self->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
        }

        self->get_transform()->set_localRotation(worldRotationQuaternion);
    }

    // if (offset.dissolve.has_value()) {
    //     CutoutAnimateEffect *cutoutAnimationEffect = ad->cutoutAnimationEffect;
    //     if (!cutoutAnimationEffect) {
    //         ObstacleDissolve *obstacleDissolve = self->get_gameObject()->GetComponent<ObstacleDissolve*>();
    //         cutoutAnimationEffect = obstacleDissolve->cutoutAnimateEffect;
    //         ad->cutoutAnimationEffect = cutoutAnimationEffect;
    //     }

    //     cutoutAnimationEffect->SetCutout(1 - *offset.dissolve);
    //     // cutoutAnimationEffect->SetCutout(0.75);
    // }

    ObstacleController_Update(self);
}

MAKE_HOOK_OFFSETLESS(ObstacleController_GetPosForTime, Vector3, ObstacleController *self, float time) {
    auto *obstacleData = (CustomJSONData::CustomObstacleData*) self->obstacleData;

    if (!obstacleData->customData->value) {
        return ObstacleController_GetPosForTime(self, time);
    }
    rapidjson::Value &customData = *obstacleData->customData->value;
    BeatmapObjectAssociatedData *ad = getAD(obstacleData->customData);

    float jumpTime = std::clamp((time - self->move1Duration) / (self->move2Duration + self->obstacleDuration), 0.0f, 1.0f);
    std::optional<Vector3> position = AnimationHelper::GetDefinitePositionOffset(ad->animationData, ad->track, jumpTime);

    if (position.has_value()) {
        Vector3 noteOffset = ad->noteOffset;
        Vector3 definitePosition = *position + noteOffset;
        definitePosition.x += ad->xOffset;
        if (time < self->move1Duration) {
            Vector3 result = Vector3::LerpUnclamped(self->startPos, self->midPos, time / self->move1Duration);
            return result + definitePosition - self->midPos;
        } else {
            return definitePosition;
        }
    }

    return ObstacleController_GetPosForTime(self, time);
}

MAKE_HOOK_OFFSETLESS(ParametricBoxFakeGlowController_OnEnable, void, Il2CppObject *self) {}

void NoodleExtensions::InstallObstacleControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, ObstacleController_Init, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "Init", 9));
    INSTALL_HOOK_OFFSETLESS(logger, ObstacleController_Update, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "ManualUpdate", 0));    
    INSTALL_HOOK_OFFSETLESS(logger, ObstacleController_GetPosForTime, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "GetPosForTime", 1));  
    // Temporary fake glow disable hook
    INSTALL_HOOK_OFFSETLESS(logger, ParametricBoxFakeGlowController_OnEnable, il2cpp_utils::FindMethodUnsafe("", "ParametricBoxFakeGlowController", "OnEnable", 0));
}