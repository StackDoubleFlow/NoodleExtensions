#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/MirroredCubeNoteController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Renderer.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "Animation/ParentObject.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

static constexpr void AddToTrack(CustomJSONData::CustomNoteData* noteData, GameObject* gameObject)
{
    if (noteData->customData && noteData->customData->value) {
        auto const &tracks = TracksAD::getAD(noteData->customData).tracks;
        if (!tracks.empty()) {
            for (auto &track: tracks) {
                track->AddGameObject(gameObject);
            }
        }
    }
}

static constexpr bool CheckSkip(Transform* noteTransform, Transform* followedNoteTransform)
{
    auto const position = followedNoteTransform->get_position();
    auto const noteTransformGameObject = noteTransform->get_gameObject();

    if (position.y < 0)
    {
        noteTransformGameObject->SetActive(false);

        return false;
    }

    noteTransformGameObject->SetActive(true);
    return true;
}

static void UpdateMirror(BeatmapObjectAssociatedData& ad,Transform* objectTransform, Transform* noteTransform,
                         Transform* followedObjectTransform,
                         NoteControllerBase* mirroredNoteController,
                         std::function<void(bool hidden)> const& setMirrorHide)
{

    GameObject* go = nullptr;

    if (ad.cutoutEffect) {
        // rekt
        setMirrorHide(ad.cutoutEffect->cutout < 1);
        if (ad.cutoutEffect->cutout < 1)
            return;

//        self->set_hide(true);
//        if (ad.cutoutEffect->cutout < 1) {
//            if (setMirrorHide) {
//                setMirrorHide(true);
//                return;
//            }
//        }

//        CutoutEffect *cutoutEffect = ad.mirroredCutoutEffect;
//        if (!cutoutEffect) {
//            go = mirroredNoteController->get_gameObject();
//
//            auto *baseNoteVisuals = go->GetComponent<BaseNoteVisuals *>();
//            CutoutAnimateEffect *cutoutAnimateEffect = baseNoteVisuals->cutoutAnimateEffect;
//            Array<CutoutEffect*>* cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
//            for (int i = 0; i < cuttoutEffects->Length(); i++) {
//                CutoutEffect *effect = cuttoutEffects->get(i);
//                if (csstrtostr(effect->get_name()) != u"NoteArrow") {
//                    cutoutEffect = effect;
//                    break;
//                }
//            }
//            ad.mirroredCutoutEffect = cutoutEffect;
//        }
//
//        if (cutoutEffect)
//            cutoutEffect->SetCutout(ad.cutoutEffect->cutout);
    }

    if (ad.disappearingArrowController) {
        DisappearingArrowControllerBase_1<MirroredCubeNoteController *> *disappearingArrowController = ad.mirroredDisappearingArrowController;
        if (!disappearingArrowController) {
            if (!go)
                go = mirroredNoteController->get_gameObject();

            disappearingArrowController = go->GetComponent<DisappearingArrowControllerBase_1<MirroredCubeNoteController *> *>();
            ad.mirroredDisappearingArrowController = disappearingArrowController;
        }

        if (disappearingArrowController)
            disappearingArrowController->SetArrowTransparency(1 - ad.disappearingArrowController->arrowCutoutEffect->cutout);
    }

    auto const followedObjectTransformLocalScale = followedObjectTransform->get_localScale();


    objectTransform->set_localScale(followedObjectTransformLocalScale);
    noteTransform->set_localScale(followedObjectTransformLocalScale);
}

MAKE_HOOK_FIND_CLASS_INSTANCE(MirroredCubeNoteController_Mirror, "", "MirroredCubeNoteController", "Mirror", void, MirroredCubeNoteController *self, ICubeNoteMirrorable *noteController) {
    MirroredCubeNoteController_Mirror(self, noteController);

    auto *followedNote = reinterpret_cast<GameNoteController *>(self->followedNote);
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(followedNote->noteData);
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    AddToTrack(customNoteData, self->get_gameObject());
}

MAKE_HOOK_FIND_CLASS_INSTANCE(MirroredCubeNoteController_UpdatePositionAndRotation, "", "MirroredCubeNoteController", "UpdatePositionAndRotation", void, MirroredCubeNoteController *self) {
    if (!CheckSkip(self->noteTransform, self->followedNoteTransform)) {
        return;
    }

    MirroredCubeNoteController_UpdatePositionAndRotation(self);

    auto *followedNote = reinterpret_cast<GameNoteController *>(self->followedNote);
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(followedNote->noteData);
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    UpdateMirror(ad, self->objectTransform, self->noteTransform,
                 self->followedObjectTransform, self,
                 [self](bool hide) {
                     self->set_hide(hide);
                 });
}

void InstallMirroredNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, MirroredCubeNoteController_Mirror);
    INSTALL_HOOK(logger, MirroredCubeNoteController_UpdatePositionAndRotation);
}

NEInstallHooks(InstallMirroredNoteControllerHooks);