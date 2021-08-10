#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"
#include "UnityEngine/Time.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "SharedUpdate.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

static NoteCutSoundEffectManager *currentSoundEffectManager;
static int lastFrame = -1;
static int cutCount = -1;
static std::vector<NoteController *> hitsoundQueue;

static bool ProcessHitSound(NoteController *noteController) {
    int frameCount = Time::get_frameCount();
    if (frameCount == lastFrame) {
        cutCount++;
    } else {
        lastFrame = frameCount;
        cutCount = 0;
    }

    return cutCount < 30;
}

MAKE_HOOK_MATCH(NoteCutSoundEffectManager_Start, &NoteCutSoundEffectManager::Start, void,
                NoteCutSoundEffectManager *self) {
    currentSoundEffectManager = self;
    NoteCutSoundEffectManager_Start(self);
}

MAKE_HOOK_MATCH(NoteCutSoundEffectManager_HandleNoteWasSpawned,
                &NoteCutSoundEffectManager::HandleNoteWasSpawned, void,
                NoteCutSoundEffectManager *self, NoteController *noteController) {
    if (!FakeNoteHelper::GetFakeNote(noteController) && ProcessHitSound(noteController)) {
        NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
    }
}

void InstallNoteCutSoundEffectManagerHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutSoundEffectManager_Start);
    INSTALL_HOOK(logger, NoteCutSoundEffectManager_HandleNoteWasSpawned);
}
NEInstallHooks(InstallNoteCutSoundEffectManagerHooks);