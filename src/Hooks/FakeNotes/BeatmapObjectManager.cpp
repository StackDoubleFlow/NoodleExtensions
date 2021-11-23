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

MAKE_HOOK_MATCH(HandleNoteControllerNoteWasMissed,
                &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void,
                BeatmapObjectManager *self, NoteController *noteController) {
    if (!FakeNoteHelper::GetFakeNote(noteController)) {
        HandleNoteControllerNoteWasMissed(self, noteController);
    }
}

// temp
template<typename T>
struct ::il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<ByRef<T>> {
    static inline const Il2CppType* get(ByRef<T> arg) {
        // ByRef types should NOT perform argument based lookups, since they could be holding a null reference.
        // ex: out
        // Instead, we should deduce the type from our held type alone.
        return ::il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<ByRef<T>>::get();
    }
};

// lazy
static const Il2CppType * NoteCutInfoT(ByRef<NoteCutInfo> info) {
    return il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<ByRef<NoteCutInfo>>::get();
//    return il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<ByRef<NoteCutInfo>>::get(info);
}

//GlobalNamespace::NoteCutCoreEffectsSpawner::
void HandleNoteWasCut(NoteCutCoreEffectsSpawner* self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    static auto ___internal__logger = ::Logger::get().WithContext("GlobalNamespace::NoteCutCoreEffectsSpawner::HandleNoteWasCut");
    static auto* ___internal__method = THROW_UNLESS((::il2cpp_utils::FindMethod(self, "HandleNoteWasCut", std::vector<Il2CppClass*>{}, ::std::vector<const Il2CppType*>{::il2cpp_utils::ExtractType(noteController), ::NoteCutInfoT(noteCutInfo)})));
    ::il2cpp_utils::RunMethodThrow<void, false>(self, ___internal__method, noteController, byref(noteCutInfo));
}

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut,
                &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void,
                BeatmapObjectManager *self, GlobalNamespace::NoteController* noteController, ByRef<GlobalNamespace::NoteCutInfo> noteCutInfo) {
    if (FakeNoteHelper::GetFakeNote(noteController) && noteCutCoreEffectsSpawner) {
        CRASH_UNLESS(*noteCutCoreEffectsSpawner);
//        NELogger::GetLogger().debug("note %p %p extract type %p", &noteCutInfo.heldRef, &classof(GlobalNamespace::NoteCutInfo)->this_arg, NoteCutInfoT(noteCutInfo));
//        NELogger::GetLogger().debug("no arg %p arg %p", il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<ByRef<NoteCutInfo>>::get(), il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<ByRef<NoteCutInfo>>::get(noteCutInfo));
        HandleNoteWasCut(*noteCutCoreEffectsSpawner, noteController, noteCutInfo);
        self->Despawn(noteController);
        return;
    }
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);
}

void InstallBeatmapObjectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, HandleNoteControllerNoteWasMissed);
    INSTALL_HOOK(logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
}

NEInstallHooks(InstallBeatmapObjectManagerHooks);