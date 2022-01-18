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
TracksAD::TracksVector noteTracks;

float noteTimeAdjust(float original, float jumpDuration) {
    if (noteTracks.empty())
        return original;

    Track* noteTrack = nullptr;

    if (noteTracks.size() > 1) {
        auto trackIt = std::find_if(noteTracks.begin(), noteTracks.end(), [](Track *track) {
            return track->properties.time.value.has_value();
        });

        if (trackIt != noteTracks.end()) {
            noteTrack = *trackIt;
        }
    } else {
        noteTrack = noteTracks.front();
    }


    if (noteTrack) {
        Property &timeProperty = noteTrack->properties.time;
        if (timeProperty.value) {
            float time = timeProperty.value->linear;
            return time * jumpDuration;
        }
    }

    return original;
}

std::unordered_map<NoteController *, ArrayW<ConditionalMaterialSwitcher *>> cachedNoteMaterialSwitchers;

void NECaches::ClearNoteCaches() {
    cachedNoteMaterialSwitchers.clear();
}

MAKE_HOOK_MATCH(NoteController_Init, &NoteController::Init, void,
                NoteController *self, GlobalNamespace::NoteData* noteData,
                float worldRotation, UnityEngine::Vector3 moveStartPos,
                UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos,
                float moveDuration, float jumpDuration, float jumpGravity,
                float endRotation, float uniformScale) {
    NoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration,
                        jumpGravity, endRotation, uniformScale);
    auto *customNoteData =
            reinterpret_cast<CustomJSONData::CustomNoteData *>(noteData);

    Transform *transform = self->get_transform();
    transform->set_localScale(
            NEVector::Vector3::one()); // This is a fix for animation due to notes being
    // recycled

    if (!customNoteData->customData)
        return;
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    ArrayW<ConditionalMaterialSwitcher *> materialSwitchers;
    auto it = cachedNoteMaterialSwitchers.find(self);
    if (it == cachedNoteMaterialSwitchers.end()) {
        cachedNoteMaterialSwitchers[self] = materialSwitchers = self->get_gameObject()->GetComponentsInChildren<ConditionalMaterialSwitcher *>();
    } else {
        materialSwitchers = it->second;
    }
    ad.materialSwitchers = materialSwitchers;
    for (auto *materialSwitcher: materialSwitchers) {
        materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material0);
    }
    ad.dissolveEnabled = false;

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    auto const &curDir = ad.objectData.cutDirection;
    if (curDir.has_value()) {
        NEVector::Quaternion cutQuaternion = curDir.value();
        noteJump->endRotation = cutQuaternion;
        NEVector::Vector3 vector = cutQuaternion.get_eulerAngles();
        vector =
                vector +
                NEVector::Vector3(noteJump->randomRotations.get(noteJump->randomRotationIdx)) * 20;
        NEVector::Quaternion midrotation = NEVector::Quaternion::Euler(vector);
        noteJump->middleRotation = midrotation;
    }

    NEVector::Quaternion localRotation = NEVector::Quaternion::identity();
    if (ad.objectData.rotation || ad.objectData.localRotation) {
        if (ad.objectData.localRotation) {
            localRotation = *ad.objectData.localRotation;
        }

        if (ad.objectData.rotation) {
            NEVector::Quaternion worldRotationQuatnerion = *ad.objectData.rotation;

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
    auto const &tracks = TracksAD::getAD(customNoteData->customData).tracks;
    if (!tracks.empty()) {
        auto go = self->get_gameObject();
        for (auto &track: tracks) {
            track->AddGameObject(go);
        }
    }
    // how fucking long has _zOffset existed???!??
    float zOffset = self->noteMovement->zOffset;
    moveStartPos.z += zOffset;
    moveEndPos.z += zOffset;
    jumpEndPos.z += zOffset;

    ad.endRotation = endRotation;
    ad.moveStartPos = moveStartPos;
    ad.moveEndPos = moveEndPos;
    ad.jumpEndPos = jumpEndPos;
    ad.worldRotation = self->get_worldRotation();
    ad.localRotation = localRotation;


//    self->Update();
}

MAKE_HOOK_MATCH(NoteController_ManualUpdate, &NoteController::ManualUpdate, void,
                NoteController *self) {

    auto *customNoteData =
        reinterpret_cast<CustomJSONData::CustomNoteData *>(self->noteData);
    if (!customNoteData->customData) {
        noteUpdateAD = nullptr;
        noteTracks.clear();
        return NoteController_ManualUpdate(self);
    }

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     NoteController_Update(self);
    //     return;
    // }

    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);
    auto const& tracks = TracksAD::getAD(customNoteData->customData).tracks;

    noteUpdateAD = &ad;
    noteTracks = tracks;

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    float songTime = TimeSourceHelper::getSongTime(noteJump->audioTimeSyncController);
    float elapsedTime = songTime - (customNoteData->time - (noteJump->jumpDuration * 0.5f));
    elapsedTime = noteTimeAdjust(elapsedTime, noteJump->jumpDuration);
    float normalTime = elapsedTime / noteJump->jumpDuration;


    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(
        ad.animationData, tracks, normalTime);

    if (offset.positionOffset.has_value()) {
        auto const& offsetPos = *offset.positionOffset;
        floorMovement->startPos = ad.moveStartPos + offsetPos;
        floorMovement->endPos = ad.moveEndPos + offsetPos;
        noteJump->startPos = ad.moveEndPos + offsetPos;
        noteJump->endPos = ad.jumpEndPos + offsetPos;
    }

    auto transform = self->get_transform();

    if (offset.scaleOffset.has_value()) {
        transform->set_localScale(*offset.scaleOffset);
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

        transform->set_localRotation(worldRotationQuaternion);
    }

    bool noteDissolveConfig = getNEConfig().enableNoteDissolve.GetValue();
    bool hasDissolveOffset = offset.dissolve.has_value() || offset.dissolveArrow.has_value();
    if (hasDissolveOffset && !ad.dissolveEnabled && noteDissolveConfig) {
        ArrayW<ConditionalMaterialSwitcher *> materialSwitchers = ad.materialSwitchers;
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
            ArrayW<CutoutEffect*> cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
            for (CutoutEffect *effect : cuttoutEffects) {
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
            ArrayW<BoxCuttableBySaber *> bigCuttables = gameNoteController->bigCuttableBySaberList;
            for (auto bigCuttable : bigCuttables) {
                if (bigCuttable->canBeCut != enabled) {
                    bigCuttable->set_canBeCut(enabled);
                }
            }
            ArrayW<BoxCuttableBySaber *> smallCuttables = gameNoteController->smallCuttableBySaberList;
            for (auto smallCuttable : smallCuttables) {
                if (smallCuttable->canBeCut != enabled) {
                    smallCuttable->set_canBeCut(enabled);
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