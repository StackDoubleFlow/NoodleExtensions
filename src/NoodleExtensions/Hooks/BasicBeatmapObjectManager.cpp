#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData_ObstacleSpawnData.hpp"
#include "GlobalNamespace/MonoMemoryPoolContainer_1.hpp"

#include "NoodleExtensions/NEHooks.h"

using namespace GlobalNamespace;

struct BeatmapObjectSpawnMovementData_ObstacleSpawnData {
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    float obstacleHeight;
    float moveDuration;
    float jumpDuration;
    float noteLinesDistance;
    constexpr BeatmapObjectSpawnMovementData_ObstacleSpawnData(UnityEngine::Vector3 moveStartPos_ = {}, UnityEngine::Vector3 moveEndPos_ = {}, UnityEngine::Vector3 jumpEndPos_ = {}, float obstacleHeight_ = {}, float moveDuration_ = {}, float jumpDuration_ = {}, float noteLinesDistance_ = {}) noexcept : moveStartPos{moveStartPos_}, moveEndPos{moveEndPos_}, jumpEndPos{jumpEndPos_}, obstacleHeight{obstacleHeight_}, moveDuration{moveDuration_}, jumpDuration{jumpDuration_}, noteLinesDistance{noteLinesDistance_} {}
};

// Why the fuck is this needed? the fuck is this?? this is a week of my life I'm never getting back, fuck this game.
MAKE_HOOK_OFFSETLESS(SpawnObstacleInternal, ObstacleController *, BasicBeatmapObjectManager *self, ObstacleData *obstacleData, BeatmapObjectSpawnMovementData_ObstacleSpawnData obstacleSpawnData, float rotation) {
    ObstacleController *obstacleController = self->obstaclePoolContainer->Spawn();
    obstacleController->Init(obstacleData, rotation, obstacleSpawnData.moveStartPos, obstacleSpawnData.moveEndPos, obstacleSpawnData.jumpEndPos, obstacleSpawnData.moveDuration, obstacleSpawnData.jumpDuration, obstacleSpawnData.noteLinesDistance, obstacleSpawnData.obstacleHeight);
    return obstacleController;
}

void NoodleExtensions::InstallBasicBeatmapObjectManagerHooks() {
    // INSTALL_HOOK_OFFSETLESS(SpawnObstacleInternal, il2cpp_utils::FindMethodUnsafe("", "BasicBeatmapObjectManager", "SpawnObstacleInternal", 3));
}