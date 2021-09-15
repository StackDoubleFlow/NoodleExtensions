#pragma once

namespace GlobalNamespace {
    class ConditionalMaterialSwitcher;
    class BaseNoteVisuals;
    class NoteControllerBase;
    class GameNoteController;
    class MirroredCubeNoteController;

    template<typename T>
    class DisappearingArrowControllerBase_1;


    class ObstacleDissolve;
    class ObstacleController;
}

class NECaches {
public:
    static float noteJumpMovementSpeed;
    static float noteJumpStartBeatOffset;

    struct NoteCache {
        Array<GlobalNamespace::ConditionalMaterialSwitcher *>* conditionalMaterialSwitchers;
        GlobalNamespace::BaseNoteVisuals* baseNoteVisuals;

        GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController *> * disappearingArrowController;
        GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredCubeNoteController *> *mirroredDisappearingArrowController;
    };

    struct ObstacleCache {
        Array<GlobalNamespace::ConditionalMaterialSwitcher *>* conditionalMaterialSwitchers;
        GlobalNamespace::ObstacleDissolve* obstacleDissolve;
    };

    static inline std::unordered_map<GlobalNamespace::NoteControllerBase *, NECaches::NoteCache> noteCache;
    static inline std::unordered_map<GlobalNamespace::ObstacleController *, NECaches::ObstacleCache> obstacleCache;

    static NECaches::NoteCache& getNoteCache(GlobalNamespace::NoteControllerBase * note) {
        auto it = noteCache.find(note);
        if (it == noteCache.end()) {
            return noteCache.emplace(note, NoteCache()).first->second;
        }

        return it->second;
    }

    static NECaches::ObstacleCache& getObstacleCache(GlobalNamespace::ObstacleController * obstacle) {
        auto it = obstacleCache.find(obstacle);
        if (it == obstacleCache.end()) {
            return obstacleCache.emplace(obstacle, ObstacleCache()).first->second;
        }

        return it->second;
    }

    static void ClearObstacleCaches() {
        obstacleCache.clear();
    }
    static void ClearNoteCaches() {
        noteCache.clear();
    }
};