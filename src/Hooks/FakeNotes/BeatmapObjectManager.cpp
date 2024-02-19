#include "beatsaber-hook/shared/utils/byref.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

extern std::optional<NoteCutCoreEffectsSpawner*> noteCutCoreEffectsSpawner;

MAKE_HOOK_MATCH(HandleNoteControllerNoteWasMissed, &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void,
                BeatmapObjectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return HandleNoteControllerNoteWasMissed(self, noteController);

  if (!FakeNoteHelper::GetFakeNote(noteController->noteData)) {
    HandleNoteControllerNoteWasMissed(self, noteController);
  }
}

// lazy
static Il2CppType const* NoteCutInfoT(ByRef<NoteCutInfo> info) {
  return il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<ByRef<NoteCutInfo>>::get();
  //    return il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<ByRef<NoteCutInfo>>::get(info);
}

// GlobalNamespace::NoteCutCoreEffectsSpawner::
void HandleNoteWasCut(NoteCutCoreEffectsSpawner* self, GlobalNamespace::NoteController* noteController,
                      ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
  static auto ___internal__logger =
      ::Logger::get().WithContext("GlobalNamespace::NoteCutCoreEffectsSpawner::HandleNoteWasCut");
  static auto* ___internal__method = THROW_UNLESS((::il2cpp_utils::FindMethod(
      self, "HandleNoteWasCut", std::vector<Il2CppClass*>{},
      ::std::vector<Il2CppType const*>{ ::il2cpp_utils::ExtractType(noteController), ::NoteCutInfoT(noteCutInfo) })));
  ::il2cpp_utils::RunMethodRethrow<void, false>(self, ___internal__method, noteController, byref(noteCutInfo));
}

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut,
                &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void, BeatmapObjectManager* self,
                GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);

  // If not fake note or noteCutCoreEffectsSpawner is null
  if (!FakeNoteHelper::GetFakeNote(noteController->noteData)) {
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);
    return;
  }

  if (!noteCutCoreEffectsSpawner) {
    NELogger::GetLogger().error("noteCutCoreEffectsSpawner is null");
    return;
  }

  CRASH_UNLESS(*noteCutCoreEffectsSpawner);
  //        NELogger::GetLogger().debug("note %p %p extract type %p", &noteCutInfo.heldRef,
  //        &classof(GlobalNamespace::NoteCutInfo)->this_arg, NoteCutInfoT(noteCutInfo));
  //        NELogger::GetLogger().debug("no arg %p arg %p",
  //        il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<ByRef<NoteCutInfo>>::get(),
  //        il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<ByRef<NoteCutInfo>>::get(noteCutInfo));
  HandleNoteWasCut(*noteCutCoreEffectsSpawner, noteController, noteCutInfo);
  self->Despawn(noteController);
}

void InstallBeatmapObjectManagerHooks(Logger& logger) {
  INSTALL_HOOK(logger, HandleNoteControllerNoteWasMissed);
  INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
}

NEInstallHooks(InstallBeatmapObjectManagerHooks);