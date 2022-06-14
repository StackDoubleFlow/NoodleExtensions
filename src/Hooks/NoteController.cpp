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

BeatmapObjectAssociatedData *noteUpdateAD = nullptr;
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

void NECaches::ClearNoteCaches() {
    NECaches::noteCache.clear();
    noteUpdateAD = nullptr;
    noteTracks.clear();
}

MAKE_HOOK_MATCH(NoteController_Init, &NoteController::Init, void,
                NoteController *self, GlobalNamespace::NoteData* noteData,
                float worldRotation, ::UnityEngine::Vector3 moveStartPos,
                ::UnityEngine::Vector3 moveEndPos, ::UnityEngine::Vector3 jumpEndPos,
                float moveDuration, float jumpDuration, float jumpGravity,
                float endRotation, float uniformScale, bool rotateTowardsPlayer, bool useRandomRotation) {
    NoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration,
                        jumpGravity, endRotation, uniformScale, rotateTowardsPlayer, useRandomRotation);

    if (!Hooks::isNoodleHookEnabled())
        return;

    auto *customNoteData =
            reinterpret_cast<CustomJSONData::CustomNoteData *>(noteData);

    Transform *transform = self->get_transform();
    transform->set_localScale(
            NEVector::Vector3::one()); // This is a fix for animation due to notes being
    // recycled

    if (!customNoteData->customData)
        return;
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    // TRANSPILERS SUCK!
    auto flipYSide = ad.flip ? ad.flip->y : customNoteData->flipYSide;

    if (flipYSide > 0.0f)
    {
        self->noteMovement->jump->yAvoidance = flipYSide * self->noteMovement->jump->yAvoidanceUp;
    }
    else
    {
        self->noteMovement->jump->yAvoidance = flipYSide * self->noteMovement->jump->yAvoidanceDown;
    }

    auto& noteCache = NECaches::getNoteCache(self);

    ArrayW<ConditionalMaterialSwitcher *>& materialSwitchers = noteCache.conditionalMaterialSwitchers;
    if (!materialSwitchers) {
         materialSwitchers = self->get_gameObject()->GetComponentsInChildren<ConditionalMaterialSwitcher *>();
    }

    for (auto *materialSwitcher: materialSwitchers) {
        materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material0);
    }

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    float zOffset = self->noteMovement->zOffset;
    moveStartPos.z += zOffset;
    moveEndPos.z += zOffset;
    jumpEndPos.z += zOffset;




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

    ad.endRotation = endRotation;
    ad.moveStartPos = moveStartPos;
    ad.moveEndPos = moveEndPos;
    ad.jumpEndPos = jumpEndPos;
    ad.worldRotation = self->get_worldRotation();
    ad.localRotation = localRotation;


    self->Update();
}

MAKE_HOOK_MATCH(NoteController_ManualUpdate, &NoteController::ManualUpdate, void,
                NoteController *self) {

    if (!Hooks::isNoodleHookEnabled())
        return NoteController_ManualUpdate(self);

    noteUpdateAD = nullptr;
    noteTracks.clear();

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

    if (noteTracks.empty() && !ad.animationData.parsed) {
        return NoteController_ManualUpdate(self);
    }

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

    auto& noteCache = NECaches::getNoteCache(self);

    bool noteDissolveConfig = getNEConfig().enableNoteDissolve.GetValue();
    bool hasDissolveOffset = offset.dissolve.has_value() || offset.dissolveArrow.has_value();
    bool isDissolving = offset.dissolve.value_or(0) > 0 || offset.dissolveArrow.value_or(0) > 0;
    if (hasDissolveOffset && ad.dissolveEnabled != isDissolving && noteDissolveConfig) {
        ArrayW<ConditionalMaterialSwitcher *> materialSwitchers = noteCache.conditionalMaterialSwitchers;
        for (auto *materialSwitcher : materialSwitchers) {
            materialSwitcher->renderer->set_sharedMaterial(isDissolving ? materialSwitcher->material1 : materialSwitcher->material0);
        }
        ad.dissolveEnabled = isDissolving;
    }

    if (offset.dissolve.has_value()) {
        CutoutEffect *& cutoutEffect = noteCache.cutoutEffect;
        if (!cutoutEffect) {
            noteCache.baseNoteVisuals = noteCache.baseNoteVisuals ?: self->get_gameObject()->GetComponent<BaseNoteVisuals *>();
            CutoutAnimateEffect *cutoutAnimateEffect = noteCache.baseNoteVisuals->cutoutAnimateEffect;
            ArrayW<CutoutEffect*> cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
            for (CutoutEffect *effect : cuttoutEffects) {
                if (effect->get_name() != u"NoteArrow") {
                    cutoutEffect = effect;
                    break;
                }
            }
        }

        CRASH_UNLESS(cutoutEffect);

        if (noteDissolveConfig) {
            cutoutEffect->SetCutout(1 - *offset.dissolve);
        } else {
            cutoutEffect->SetCutout(*offset.dissolve >= 0 ? 0 : 1);
        }
    }

    if (offset.dissolveArrow.has_value() && self->noteData->colorType != ColorType::None) {
        auto& disappearingArrowController = noteCache.disappearingArrowController;
        if (!disappearingArrowController) {
            disappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController *> *>();
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

    // NoteJump.ManualUpdate will be the last place this is used after it was set in
    // NoteController.ManualUpdate. To make sure it doesn't interfere with future notes, it's set
    // back to null
    noteUpdateAD = nullptr;
    noteTracks.clear();
}

void InstallNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteController_Init);
    INSTALL_HOOK(logger, NoteController_ManualUpdate);
}

NEInstallHooks(InstallNoteControllerHooks);