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
#include "NECaches.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_FIND_CLASS_INSTANCE(MirroredCubeNoteController_Mirror, "", "MirroredCubeNoteController", "Mirror", void, MirroredCubeNoteController *self, ICubeNoteMirrorable *noteController) {
    self->followedNote = noteController;
    self->followedObjectTransform = reinterpret_cast<INoteMirrorable *>(self->followedNote)->get_transform();
    self->followedNoteTransform = reinterpret_cast<INoteMirrorable *>(self->followedNote)->get_noteTransform();
    if (self->followedNoteTransform->get_position().y < 0) {
        if (self->noteTransform->get_gameObject()->get_activeInHierarchy()) {
            self->noteTransform->get_gameObject()->set_active(false);    
        }
        return;
    } else if (!self->noteTransform->get_gameObject()->get_activeInHierarchy()) {
        self->noteTransform->get_gameObject()->set_active(true);
    }

    MirroredCubeNoteController_Mirror(self, noteController);

    self->objectTransform->set_localScale(self->followedObjectTransform->get_localScale());
    self->noteTransform->set_localScale(self->followedNoteTransform->get_localScale());

    auto *followedNote = reinterpret_cast<GameNoteController *>(self->followedNote);
    auto *customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData *>(followedNote->noteData);
    BeatmapObjectAssociatedData &ad = getAD(customNoteData->customData);

    // if (ad.cutoutEffect || ad.disappearingArrowController) {
    //     Renderer *renderer = ad.mirroredRenderer;
    //     if (!renderer) {
    //         renderer = self->get_gameObject()->GetComponentInChildren<Renderer *>();
    //         renderer->set_sharedMaterial(ad.materialSwitcher->material1);
    //     }
    // }

    auto& noteCache = NECaches::getNoteCache(self);
    if (ad.cutoutEffect) {
        // rekt
        self->set_hide(true);

        CutoutEffect *cutoutEffect = ad.mirroredCutoutEffect;
        if (!cutoutEffect) {
            BaseNoteVisuals *baseNoteVisuals = noteCache.baseNoteVisuals;
            if (!baseNoteVisuals)
                baseNoteVisuals = noteCache.baseNoteVisuals = self->get_gameObject()->GetComponent<BaseNoteVisuals *>();
            CutoutAnimateEffect *cutoutAnimateEffect = baseNoteVisuals->cutoutAnimateEffect;
            Array<CutoutEffect*>* cuttoutEffects = cutoutAnimateEffect->cuttoutEffects;
            for (int i = 0; i < cuttoutEffects->Length(); i++) {
                CutoutEffect *effect = cuttoutEffects->get(i);
                if (csstrtostr(effect->get_name()) != u"NoteArrow") {
                    cutoutEffect = effect;
                    break;
                }
            }
            ad.mirroredCutoutEffect = cutoutEffect;
        }

        cutoutEffect->SetCutout(ad.cutoutEffect->cutout);
    }

    if (ad.disappearingArrowController) {
        DisappearingArrowControllerBase_1<MirroredCubeNoteController *> *disappearingArrowController = noteCache.mirroredDisappearingArrowController;

        if (!disappearingArrowController) {
            disappearingArrowController = noteCache.mirroredDisappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<MirroredCubeNoteController *> *>();
            ad.mirroredDisappearingArrowController = disappearingArrowController;
        }

        disappearingArrowController->SetArrowTransparency(1 - ad.disappearingArrowController->arrowCutoutEffect->cutout);
    }
}

void InstallMirroredNoteControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, MirroredCubeNoteController_Mirror);
}

NEInstallHooks(InstallMirroredNoteControllerHooks);