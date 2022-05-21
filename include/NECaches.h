#include "GlobalNamespace/BeatmapObjectSpawnMovementData_NoteJumpValueType.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"

class NECaches {
public:
static float noteJumpMovementSpeed;
static float noteJumpStartBeatOffset;
static float beatsPerMinute;
static float numberOfLines;
static GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType;
static float noteJumpValue;

static bool LeftHandedMode;

static void ClearObstacleCaches();
static void ClearNoteCaches();
};