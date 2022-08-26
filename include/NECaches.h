#pragma once
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteJumpValueType.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"
#include "sombrero/shared/ColorUtils.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController_InitData.hpp"
#include "Zenject/DiContainer.hpp"

#pragma once

namespace GlobalNamespace {
    class ConditionalMaterialSwitcher;
    class BaseNoteVisuals;
    class NoteControllerBase;
    class GameNoteController;
    class MirroredGameNoteController;
    class CutoutEffect;

    template<typename T>
    class DisappearingArrowControllerBase_1;


    class ObstacleDissolve;
    class ObstacleControllerBase;
    class CutoutAnimateEffect;
}

class NECaches {
public:
static float noteJumpMovementSpeed;
static float noteJumpStartBeatOffset;
static float beatsPerMinute;
static float numberOfLines;
static SafePtr<Zenject::DiContainer> GameplayCoreContainer;
static GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType;
static float noteJumpValue;

static bool LeftHandedMode;

struct NoteCache {
    ArrayW<GlobalNamespace::ConditionalMaterialSwitcher *> conditionalMaterialSwitchers;
    GlobalNamespace::BaseNoteVisuals* baseNoteVisuals{};
    GlobalNamespace::CutoutEffect* cutoutEffect{};

    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController *> * disappearingArrowController{};
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredGameNoteController *> *mirroredDisappearingArrowController{};
    bool dissolveEnabled;
};

struct ObstacleCache {
    ArrayW<GlobalNamespace::ConditionalMaterialSwitcher *> conditionalMaterialSwitchers;
    GlobalNamespace::ObstacleDissolve* obstacleDissolve{};
    GlobalNamespace::CutoutAnimateEffect* cutoutAnimateEffect{};
    std::optional<Sombrero::FastColor> color;
    void* cachedData; // just an id
    bool dissolveEnabled;
};

static inline std::unordered_map<GlobalNamespace::NoteControllerBase *, NECaches::NoteCache> noteCache;
static inline std::unordered_map<GlobalNamespace::ObstacleControllerBase *, NECaches::ObstacleCache> obstacleCache;

static inline NECaches::NoteCache& getNoteCache(GlobalNamespace::NoteControllerBase * note) {
    return noteCache[note];
}

static inline NECaches::ObstacleCache& getObstacleCache(GlobalNamespace::ObstacleControllerBase * obstacle) {
    return obstacleCache[obstacle];
}

static void ClearObstacleCaches();
static void ClearNoteCaches();

consteval static auto get_noteLinesDistanceFast() {
    return 0.6f;
}

    static GlobalNamespace::CutoutEffect* GetCutout(GlobalNamespace::NoteControllerBase* nc, NECaches::NoteCache& noteCache);


    static GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController *>* GetDisappearingArrowController(GlobalNamespace::GameNoteController* nc, NECaches::NoteCache& noteCache);
    static GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredGameNoteController *>* GetDisappearingArrowController(GlobalNamespace::MirroredGameNoteController* nc, NECaches::NoteCache& noteCache);
};