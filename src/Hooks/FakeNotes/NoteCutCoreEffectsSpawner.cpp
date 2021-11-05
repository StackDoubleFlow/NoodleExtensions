#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

std::optional<NoteCutCoreEffectsSpawner*> noteCutCoreEffectsSpawner;

MAKE_HOOK_MATCH(NoteCutCoreEffectsSpawner_Start, &NoteCutCoreEffectsSpawner::Start, void,
                NoteCutCoreEffectsSpawner *self) {
    NoteCutCoreEffectsSpawner_Start(self);
    if (self)
        noteCutCoreEffectsSpawner.emplace(self);
}

MAKE_HOOK_MATCH(NoteCutCoreEffectsSpawner_OnDestroy, &NoteCutCoreEffectsSpawner::OnDestroy, void,
                NoteCutCoreEffectsSpawner *self) {
    NoteCutCoreEffectsSpawner_OnDestroy(self);
    noteCutCoreEffectsSpawner = std::nullopt;
}

void InstallNoteCutCoreEffectsSpawnerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutCoreEffectsSpawner_Start);
    INSTALL_HOOK(logger, NoteCutCoreEffectsSpawner_OnDestroy);
}
NEInstallHooks(InstallNoteCutCoreEffectsSpawnerHooks);