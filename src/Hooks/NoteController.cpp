#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "NEConfig.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "TimeSourceHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

BeatmapObjectAssociatedData *noteUpdateAD;

float noteTimeAdjust(float original, float jumpDuration) {
    if (noteUpdateAD->track) {
        Property &timeProperty = noteUpdateAD->track->properties.time;
        if (timeProperty.value) {
            float time = timeProperty.value->linear;
            return time * jumpDuration;
        }
    }

    return original;
}

MAKE_HOOK_MATCH(NoteController_Init, &NoteController::Init, void,
                NoteController *self, NoteData *noteData, float worldRotation,
                Vector3 startPos, Vector3 midPos, Vector3 endPos,
                float move1Duration, float move2Duration, float jumpGravity,
                float endRotation, float uniformScale) {
    NoteController_Init(self, noteData, worldRotation, startPos, midPos, endPos,
                        move1Duration, move2Duration, jumpGravity, endRotation,
                        uniformScale);
    auto *customNoteData =
        reinterpret_cast<CustomJSONData::CustomNoteData *>(noteData);

    Transform *transform = self->get_transform();
    transform->set_localScale(
        Vector3::get_one()); // This is a fix for animation due to notes being
                             // recycled

    if (!customNoteData->customData->value) {
        return;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    std::optional<float> curDir =
        customData.HasMember("_cutDirection")
            ? std::optional{customData["_cutDirection"].GetFloat()}
            : std::nullopt;

    if (curDir.has_value()) {
        Quaternion cutQuaternion = Quaternion::Euler(0, 0, curDir.value());
        noteJump->endRotation = cutQuaternion;
        Vector3 vector = cutQuaternion.get_eulerAngles();
        vector =
            vector +
            noteJump->randomRotations->values[noteJump->randomRotationIdx] * 20;
        Quaternion midrotation = Quaternion::Euler(vector);
        noteJump->middleRotation = midrotation;
    }

    Quaternion localRotation = Quaternion::get_identity();
    if (customData.HasMember("_rotation") ||
        customData.HasMember("_localRotation")) {
        if (customData.HasMember("_localRotation")) {
            float x = customData["_localRotation"][0].GetFloat();
            float y = customData["_localRotation"][1].GetFloat();
            float z = customData["_localRotation"][2].GetFloat();
            localRotation = Quaternion::Euler(x, y, z);
        }

        Quaternion worldRotationQuatnerion;
        if (customData.HasMember("_rotation")) {
            if (customData["_rotation"].IsArray()) {
                float x = customData["_rotation"][0].GetFloat();
                float y = customData["_rotation"][1].GetFloat();
                float z = customData["_rotation"][2].GetFloat();
                worldRotationQuatnerion = Quaternion::Euler(x, y, z);
            } else {
                worldRotationQuatnerion =
                    Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }

            Quaternion inverseWorldRotation =
                Quaternion::Euler(-worldRotationQuatnerion.get_eulerAngles());
            noteJump->worldRotation = worldRotationQuatnerion;
            noteJump->inverseWorldRotation = inverseWorldRotation;
            floorMovement->worldRotation = worldRotationQuatnerion;
            floorMovement->inverseWorldRotation = inverseWorldRotation;

            worldRotationQuatnerion = worldRotationQuatnerion * localRotation;
            transform->set_localRotation(worldRotationQuatnerion);
        } else {
            transform->set_localRotation(localRotation);
        }
    }

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

    ad.moveStartPos = startPos;
    ad.moveEndPos = midPos;
    ad.jumpEndPos = endPos;
    ad.worldRotation = self->get_worldRotation();
    ad.localRotation = localRotation;

    self->Update();
}

MAKE_HOOK_MATCH(NoteController_Update, &NoteController::Update, void,
                NoteController *self) {

    auto *customNoteData =
        reinterpret_cast<CustomJSONData::CustomNoteData *>(self->noteData);
    if (!customNoteData->customData->value) {
        NoteController_Update(self);
        return;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     NoteController_Update(self);
    //     return;
    // }
    rapidjson::Value &animation = customData["_animation"];

    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    noteUpdateAD = &ad;

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    float songTime = TimeSourceHelper::getSongTime(noteJump->audioTimeSyncController);
    float elapsedTime =
        songTime - (noteJump->beatTime - (noteJump->jumpDuration * 0.5));
    elapsedTime = noteTimeAdjust(elapsedTime, noteJump->jumpDuration);
    float normalTime = elapsedTime / noteJump->jumpDuration;

    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(
        ad.animationData, ad.track, normalTime);

    if (offset.positionOffset.has_value()) {
        floorMovement->startPos = ad.moveStartPos + *offset.positionOffset;
        floorMovement->endPos = ad.moveEndPos + *offset.positionOffset;
        noteJump->startPos = ad.moveEndPos + *offset.positionOffset;
        noteJump->endPos = ad.jumpEndPos + *offset.positionOffset;
    }

    if (offset.scaleOffset.has_value()) {
        self->get_transform()->set_localScale(*offset.scaleOffset);
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
            noteJump->worldRotation = worldRotationQuaternion;
            noteJump->inverseWorldRotation = inverseWorldRotation;
            floorMovement->worldRotation = worldRotationQuaternion;
            floorMovement->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion =
                worldRotationQuaternion * *offset.localRotationOffset;
        }

        self->get_transform()->set_localRotation(worldRotationQuaternion);
    }

    if (getNEConfig().enableNoteDissolve.GetValue()) {
        if (offset.dissolve.has_value() || offset.dissolveArrow.has_value()) {
            ConditionalMaterialSwitcher *materialSwitcher = ad.materialSwitcher;
            if (!materialSwitcher) {
                materialSwitcher = self->get_gameObject()->GetComponentInChildren<ConditionalMaterialSwitcher *>();
                ad.materialSwitcher = materialSwitcher;
            }
            if (!materialSwitcher->value->get_value()) {
                materialSwitcher->value->set_value(true);
            }
        }
    }
    

    if (offset.dissolve.has_value() && getNEConfig().enableNoteDissolve.GetValue()) {
        CutoutEffect *cutoutEffect = ad.cutoutEffect;
        if (!cutoutEffect) {
            BaseNoteVisuals *baseNoteVisuals = self->get_gameObject()->GetComponent<BaseNoteVisuals *>();
            CutoutAnimateEffect *cutoutAnimateEffect = baseNoteVisuals->cutoutAnimateEffect;
            Array<CutoutEffect*>* cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
            for (int i = 0; i < cuttoutEffects->Length(); i++) {
                CutoutEffect *effect = cuttoutEffects->get(i);
                if (to_utf8(csstrtostr(effect->get_name())) != "NoteArrow") {
                    cutoutEffect = effect;
                    break;
                }
            }
            ad.cutoutEffect = cutoutEffect;
        }

        cutoutEffect->SetCutout(1 - *offset.dissolve);
    }

    if (offset.dissolveArrow.has_value() && self->noteData->colorType != ColorType::None && getNEConfig().enableNoteDissolve.GetValue()) {
        DisappearingArrowControllerBase_1<GameNoteController *> *disappearingArrowController = ad.disappearingArrowController;
        if (!disappearingArrowController) {
            disappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController *> *>();
            ad.disappearingArrowController = disappearingArrowController;
        }

        disappearingArrowController->SetArrowTransparency(*offset.dissolveArrow);
    }

    NoteController_Update(self);
}

void InstallNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteController_Init);
    INSTALL_HOOK(logger, NoteController_Update);
}

NEInstallHooks(InstallNoteControllerHooks);