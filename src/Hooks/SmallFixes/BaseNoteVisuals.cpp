#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"
#include "NECaches.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(BaseNoteVisuals_Awake, &BaseNoteVisuals::Awake, void, BaseNoteVisuals* self) {
  if (!Hooks::isNoodleHookEnabled()) return BaseNoteVisuals_Awake(self);

  BaseNoteVisuals_Awake(self);

  static auto ICubeNoteTypeProviderKlass = classof(GameNoteController*); // classof(INoteMovementProvider*);
  static auto CustomNoteDataKlass = classof(CustomJSONData::CustomNoteData*);

  if (self && self->noteController &&
      il2cpp_functions::class_is_assignable_from(ICubeNoteTypeProviderKlass, self->noteController->klass)) {
    NoteController* noteController = static_cast<NoteController*>(self->noteController);

    if (!noteController->noteData) return;

    DisappearingArrowControllerBase_1<GameNoteController*>* disappearingArrowController;

    if (il2cpp_functions::class_is_assignable_from(CustomNoteDataKlass, noteController->noteData->klass)) {
      //        if (noteData->customData) {
      auto& ad = NECaches::getNoteCache(noteController);

      disappearingArrowController =
          (DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController*>*)ad.disappearingArrowController;
      if (!disappearingArrowController) {
        disappearingArrowController =
            self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController*>*>();
        ad.disappearingArrowController = disappearingArrowController;
      }
    } else {
      disappearingArrowController =
          self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController*>*>();
    }

    disappearingArrowController->SetArrowTransparency(
        1); // i have no fucking idea how this fixes the weird ghost arrow bug
  }
}

void InstallBaseNoteVisualsHooks(Logger& logger) {
  INSTALL_HOOK(logger, BaseNoteVisuals_Awake);
}

NEInstallHooks(InstallBaseNoteVisualsHooks);