#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/ICubeNoteTypeProvider.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"


using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(BaseNoteVisuals_Awake,
                &BaseNoteVisuals::Awake, void,
                BaseNoteVisuals *self) {
    BaseNoteVisuals_Awake(self);

    if (il2cpp_utils::AssignableFrom<ICubeNoteTypeProvider*>(self->noteController->klass)) {
        NoteController* noteController = static_cast<NoteController *>(self->noteController);

        if (!noteController->noteData)
            return;

        DisappearingArrowControllerBase_1<GameNoteController *> *disappearingArrowController;

        if (il2cpp_utils::AssignableFrom<CustomJSONData::CustomNoteData*>(noteController->noteData->klass)) {
//        if (noteData->customData) {
            CustomJSONData::CustomNoteData* noteData = static_cast<CustomJSONData::CustomNoteData *>(noteController->noteData);
            auto &ad = getAD(noteData->customData);

            disappearingArrowController = ad.disappearingArrowController;
            if (!disappearingArrowController) {
                disappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController *> *>();
                ad.disappearingArrowController = disappearingArrowController;
            }
        } else {
            disappearingArrowController = self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController *> *>();
        }


        disappearingArrowController->SetArrowTransparency(1); // i have no fucking idea how this fixes the weird ghost arrow bug
    }
}

void InstallBaseNoteVisualsHooks(Logger &logger) {
    INSTALL_HOOK(logger, BaseNoteVisuals_Awake);
}

NEInstallHooks(InstallBaseNoteVisualsHooks);