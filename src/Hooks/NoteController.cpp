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
#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/BoxCuttableBySaber.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "NEConfig.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

BeatmapObjectAssociatedData *noteUpdateAD;
std::vector<Track*> noteTracks;

float noteTimeAdjust(float original, float jumpDuration) {


    auto trackIt = std::find_if(noteTracks.begin(), noteTracks.end(), [](Track* track) {
        return track->properties.time.value.has_value();
    });

    if (trackIt != noteTracks.end()) {
        Track* noteTrack = *trackIt;
        Property &timeProperty = noteTrack->properties.time;
        if (timeProperty.value) {
            float time = timeProperty.value->linear;
            return time * jumpDuration;
        }
    }

    return original;
}

std::unordered_map<NoteController *, Array<ConditionalMaterialSwitcher *> *> cachedNoteMaterialSwitchers;

void NECaches::ClearNoteCaches() {
    cachedNoteMaterialSwitchers.clear();
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
        NEVector::Vector3::one()); // This is a fix for animation due to notes being
                             // recycled

    if (!customNoteData->customData->value) {
        return;
    }
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    std::optional<float> &curDir = ad.objectData.cutDirection;
    if (curDir.has_value()) {
        NEVector::Quaternion cutQuaternion = NEVector::Quaternion::Euler(0, 0, curDir.value());
        noteJump->endRotation = cutQuaternion;
        NEVector::Vector3 vector = cutQuaternion.get_eulerAngles();
        vector =
            vector +
                    NEVector::Vector3(noteJump->randomRotations->values[noteJump->randomRotationIdx]) * 20;
        NEVector::Quaternion midrotation = NEVector::Quaternion::Euler(vector);
        noteJump->middleRotation = midrotation;
    }

    NEVector::Quaternion localRotation = NEVector::Quaternion::get_identity();
    if (ad.objectData.rotation || ad.objectData.localRotation) {
        if (ad.objectData.localRotation) {
            localRotation = NEVector::Quaternion::Euler(*ad.objectData.localRotation);
        }

        if (ad.objectData.rotation) {
            NEVector::Quaternion worldRotationQuatnerion = NEVector::Quaternion::Euler(*ad.objectData.rotation);

            NEVector::Quaternion inverseWorldRotation = NEVector::Quaternion::Inverse(worldRotationQuatnerion);
            noteJump->worldRotation = worldRotationQuatnerion;
            noteJump->inverseWorldRotation = inverseWorldRotation;
            floorMovement->worldRotation = worldRotationQuatnerion;
            floorMovement->inverseWorldRotation = inverseWorldRotation;

            worldRotationQuatnerion = worldRotationQuatnerion * localRotation;
            transform->set_localRotation(worldRotationQuatnerion);
        } else {
            transform->set_localRotation(NEVector::Quaternion(transform->get_localRotation()) * localRotation);
        }
    }
    // TODO: Multi-track
    std::vector<Track *> const& tracks = TracksAD::getAD(customNoteData->customData).tracks;
    if (!tracks.empty()) {
        for (auto& track : tracks) {
            track->AddGameObject(self->get_gameObject());
        }
    }
    ad.endRotation = endRotation;
    ad.moveStartPos = startPos;
    ad.moveEndPos = midPos;
    ad.jumpEndPos = endPos;
    ad.worldRotation = self->get_worldRotation();
    ad.localRotation = localRotation;

    Array<ConditionalMaterialSwitcher *>* materialSwitchers;
    auto it = cachedNoteMaterialSwitchers.find(self);
    if (it == cachedNoteMaterialSwitchers.end()) {
        cachedNoteMaterialSwitchers[self] = materialSwitchers = self->get_gameObject()->GetComponentsInChildren<ConditionalMaterialSwitcher *>();
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

    self->Update();
}

MAKE_HOOK_MATCH(NoteController_ManualUpdate, &NoteController::ManualUpdate, void,
                NoteController *self) {

    auto *customNoteData =
        reinterpret_cast<CustomJSONData::CustomNoteData *>(self->noteData);
    if (!customNoteData->customData->value) {
        NoteController_ManualUpdate(self);
        return;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     NoteController_Update(self);
    //     return;
    // }

    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    std::vector<Track *> const& tracks = TracksAD::getAD(customNoteData->customData).tracks;
    // TODO: Multi track
    Track* track = nullptr;

    if (!tracks.empty()) {
        track = tracks.front();

        if (tracks.size() > 1) {
            NELogger::GetLogger().error("Multi tracks detected! Not supported yet, using first track");
        }
    }

    noteUpdateAD = &ad;
    noteTracks = tracks;

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    float songTime = TimeSourceHelper::getSongTime(noteJump->audioTimeSyncController);
    float elapsedTime =
        songTime - (noteJump->beatTime - (noteJump->jumpDuration * 0.5));
    elapsedTime = noteTimeAdjust(elapsedTime, noteJump->jumpDuration);
    float normalTime = elapsedTime / noteJump->jumpDuration;


    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(
        ad.animationData, tracks, normalTime);

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
        NEVector::Quaternion worldRotation = ad.worldRotation;
        NEVector::Quaternion localRotation = ad.localRotation;

        NEVector::Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion =
                worldRotationQuaternion * *offset.rotationOffset;
            NEVector::Quaternion inverseWorldRotation =
                    NEVector::Quaternion::Inverse(worldRotationQuaternion);
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

    bool noteDissolveConfig = getNEConfig().enableNoteDissolve.GetValue();
    bool hasDissolveOffset = offset.dissolve.has_value() || offset.dissolveArrow.has_value();
    if (hasDissolveOffset && !ad.dissolveEnabled && noteDissolveConfig) {
        ArrayWrapper<ConditionalMaterialSwitcher *> materialSwitchers = ad.materialSwitchers;
        for (auto *materialSwitcher : materialSwitchers) {
            materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material1);
        }
        ad.dissolveEnabled = true;
    }

    if (offset.dissolve.has_value()) {
        CutoutEffect *cutoutEffect = ad.cutoutEffect;
        if (!cutoutEffect) {
            BaseNoteVisuals *baseNoteVisuals = self->get_gameObject()->GetComponent<BaseNoteVisuals *>();
            CutoutAnimateEffect *cutoutAnimateEffect = baseNoteVisuals->cutoutAnimateEffect;
            Array<CutoutEffect*>* cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
            for (int i = 0; i < cuttoutEffects->Length(); i++) {
                CutoutEffect *effect = cuttoutEffects->get(i);
                if (csstrtostr(effect->get_name()) != u"NoteArrow") {
                    cutoutEffect = effect;
                    break;
                }
            }
            ad.cutoutEffect = cutoutEffect;
        }

        if (noteDissolveConfig) {
            cutoutEffect->SetCutout(1 - *offset.dissolve);
        } else {
            cutoutEffect->SetCutout(*offset.dissolve >= 0 ? 0 : 1);
        }
    }

    if (offset.dissolveArrow.has_value() && self->noteData->colorType != ColorType::None) {
        DisappearingArrowControllerBase_1<GameNoteController *> *disappearingArrowController = ad.disappearingArrowController;
        if (!disappearingArrowController) {
            disappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController *> *>();
            ad.disappearingArrowController = disappearingArrowController;
        }

        if (noteDissolveConfig) {
            disappearingArrowController->SetArrowTransparency(*offset.dissolveArrow);
        } else {
            disappearingArrowController->SetArrowTransparency(*offset.dissolveArrow >= 0 ? 1 : 0);
        }
    }

    static auto *gameNoteControllerClass = classof(GameNoteController *);
    static auto *bombNoteControllerClass = classof(BombNoteController *);
    if (offset.cuttable.has_value()) {
        bool enabled = *offset.cuttable >= 1;

        if (self->klass == gameNoteControllerClass) {
            auto *gameNoteController = reinterpret_cast<GameNoteController *>(self);
            Array<BoxCuttableBySaber *> *bigCuttable = gameNoteController->bigCuttableBySaberList;
            for (int i = 0; i < bigCuttable->Length(); i++) {
                if (bigCuttable->values[i]->canBeCut != enabled) {
                    bigCuttable->values[i]->set_canBeCut(enabled);
                }
            }
            Array<BoxCuttableBySaber *> *smallCuttable = gameNoteController->smallCuttableBySaberList;
            for (int i = 0; i < smallCuttable->Length(); i++) {
                if (smallCuttable->values[i]->canBeCut != enabled) {
                    smallCuttable->values[i]->set_canBeCut(enabled);
                }
            }
        } else if(self->klass == bombNoteControllerClass) {
            auto *bombNoteController = reinterpret_cast<BombNoteController *>(self);
            CuttableBySaber *cuttable = bombNoteController->cuttableBySaber;
            if (cuttable->get_canBeCut() != enabled) {
                cuttable->set_canBeCut(enabled);
            }
        }
    }

    NoteController_ManualUpdate(self);
}

void InstallNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteController_Init);
    INSTALL_HOOK(logger, NoteController_ManualUpdate);
}

NEInstallHooks(InstallNoteControllerHooks);