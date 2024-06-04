#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/ValueType.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "SpawnDataHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include <cmath>

using namespace GlobalNamespace;
using namespace NEVector;

BeatmapObjectSpawnController* beatmapObjectSpawnController;

MAKE_HOOK_MATCH(BeatmapObjectSpawnController_Start, &BeatmapObjectSpawnController::Start, void,
                BeatmapObjectSpawnController* self) {
  beatmapObjectSpawnController = self;
  BeatmapObjectSpawnController_Start(self);
}

MAKE_HOOK_MATCH(GetSliderSpawnData, &BeatmapObjectSpawnMovementData::GetSliderSpawnData,
                BeatmapObjectSpawnMovementData::SliderSpawnData, BeatmapObjectSpawnMovementData* self,
                SliderData* normalSliderData) {

  if (!Hooks::isNoodleHookEnabled()) return GetSliderSpawnData(self, normalSliderData);

  if (!il2cpp_utils::AssignableFrom<CustomJSONData::CustomSliderData*>(normalSliderData->klass))
    return GetSliderSpawnData(self, normalSliderData);

  auto* sliderData = reinterpret_cast<CustomJSONData::CustomSliderData*>(normalSliderData);
  auto result = GetSliderSpawnData(self, normalSliderData);

  // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
  if (!sliderData->customData->value) {
    return result;
  }
  BeatmapObjectAssociatedData const& ad = getAD(sliderData->customData);

  auto njs = ad.objectData.noteJumpMovementSpeed;
  auto spawnoffset = ad.objectData.noteJumpStartBeatOffset;

  bool gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

  float offset = self->noteLinesCount / 2.f;
  float headLineIndex = ad.objectData.startX ? *ad.objectData.startX + offset : sliderData->headLineIndex;
  float headLineLayer = ad.objectData.startY.value_or(sliderData->headLineLayer.value__);
  float headStartlinelayer = ad.startNoteLineLayer;
  float tailLineIndex = ad.objectData.tailStartX ? *ad.objectData.tailStartX + offset : sliderData->tailLineIndex;
  float tailLineLayer = ad.objectData.tailStartY.value_or(sliderData->tailLineLayer.value__);
  float tailStartlinelayer = ad.tailStartNoteLineLayer;

  Vector3 headOffset =
      SpawnDataHelper::GetNoteOffset(self, headLineIndex, gravityOverride ? headLineLayer : headStartlinelayer);
  Vector3 tailOffset =
      SpawnDataHelper::GetNoteOffset(self, tailLineIndex, gravityOverride ? tailLineLayer : tailStartlinelayer);

  float jumpDuration;
  float jumpDistance;
  Vector3 moveStartPos;
  Vector3 moveEndPos;
  Vector3 jumpEndPos;

  SpawnDataHelper::GetNoteJumpValues(beatmapObjectSpawnController->_initData, self, njs, spawnoffset, jumpDuration,
                                     jumpDistance, moveStartPos, moveEndPos, jumpEndPos);

  float headJumpGravity;
  float headNoGravity;

  SpawnDataHelper::NoteJumpGravityForLineLayer(self, headLineLayer, headStartlinelayer, jumpDistance, njs,
                                               headJumpGravity, headNoGravity);

  float tailJumpGravity;
  float tailNoGravity;

  SpawnDataHelper::NoteJumpGravityForLineLayer(self, tailLineLayer, tailStartlinelayer, jumpDistance, njs,
                                               tailJumpGravity, tailNoGravity);

  result = BeatmapObjectSpawnMovementData::SliderSpawnData(
      moveStartPos + headOffset, moveEndPos + headOffset, jumpEndPos + headOffset,
      gravityOverride ? headNoGravity : headJumpGravity, moveStartPos + tailOffset, moveEndPos + tailOffset,
      jumpEndPos + tailOffset, gravityOverride ? tailNoGravity : tailJumpGravity, self->moveDuration, jumpDuration);

  return result;
}

MAKE_HOOK_MATCH(GetObstacleSpawnData, &BeatmapObjectSpawnMovementData::GetObstacleSpawnData,
                BeatmapObjectSpawnMovementData::ObstacleSpawnData, BeatmapObjectSpawnMovementData* self,
                ObstacleData* normalObstacleData) {
  if (!Hooks::isNoodleHookEnabled()) return GetObstacleSpawnData(self, normalObstacleData);

  if (!il2cpp_utils::AssignableFrom<CustomJSONData::CustomObstacleData*>(normalObstacleData->klass))
    return GetObstacleSpawnData(self, normalObstacleData);

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(normalObstacleData);
  BeatmapObjectSpawnMovementData::ObstacleSpawnData result = GetObstacleSpawnData(self, obstacleData);

  // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
  if (!obstacleData->customData) {
    return result;
  }
  BeatmapObjectAssociatedData const& ad = getAD(obstacleData->customData);

  float lineIndex =
      ad.objectData.startX ? (*ad.objectData.startX + (self->noteLinesCount / 2.f)) : obstacleData->lineIndex;
  float lineLayer = ad.objectData.startY.value_or(obstacleData->lineLayer.value__);

  std::optional<float> const& njs = ad.objectData.noteJumpMovementSpeed;
  std::optional<float> const& spawnOffset = ad.objectData.noteJumpStartBeatOffset;

  auto const& scale = ad.objectData.scale;
  std::optional<float> height = scale && scale->at(1) ? scale->at(1) : std::nullopt;
  std::optional<float> width = scale && scale->at(0) ? scale->at(0) : std::nullopt;

  Vector3 obstacleOffset = SpawnDataHelper::GetObstacleOffset(self, lineIndex, lineLayer);
  obstacleOffset.y += self->get_jumpOffsetY();

  // original code has this line, not sure how important it is
  ////obstacleOffset.y = Mathf.Max(obstacleOffset.y, this._verticalObstaclePosY);

  float obstacleHeight;
  if (height.has_value()) {
    obstacleHeight = height.value() * 0.6f;
  } else {
    // _topObstaclePosY =/= _obstacleTopPosY
    obstacleHeight = std::min(obstacleData->height * 0.6f, self->_obstacleTopPosY - obstacleOffset.y);
  }

  float jumpDuration;
  float jumpDistance;
  Vector3 localMoveStartPos;
  Vector3 localMoveEndPos;
  Vector3 localJumpEndPos;
  SpawnDataHelper::GetNoteJumpValues(beatmapObjectSpawnController->_initData, self, njs, spawnOffset, jumpDuration,
                                     jumpDistance, localMoveStartPos, localMoveEndPos, localJumpEndPos);

  result = BeatmapObjectSpawnMovementData::ObstacleSpawnData(
      localMoveStartPos + obstacleOffset, localMoveEndPos + obstacleOffset, localJumpEndPos + obstacleOffset,
      obstacleHeight, result.moveDuration, jumpDuration, NECaches::get_noteLinesDistanceFast());

  return result;
}

MAKE_HOOK_MATCH(GetJumpingNoteSpawnData, &BeatmapObjectSpawnMovementData::GetJumpingNoteSpawnData,
                BeatmapObjectSpawnMovementData::NoteSpawnData, BeatmapObjectSpawnMovementData* self,
                NoteData* normalNoteData) {
  if (!Hooks::isNoodleHookEnabled()) return GetJumpingNoteSpawnData(self, normalNoteData);

  auto noteDataCast = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(normalNoteData);
  if (!noteDataCast) return GetJumpingNoteSpawnData(self, normalNoteData);

  auto noteData = *noteDataCast;
  if (!noteData->customData) {
    return GetJumpingNoteSpawnData(self, normalNoteData);
  }

  BeatmapObjectAssociatedData& ad = getAD(noteData->customData);

  float offset = self->noteLinesCount / 2.0f;

  auto const njs = ad.objectData.noteJumpMovementSpeed;
  auto const spawnOffset = ad.objectData.noteJumpStartBeatOffset;
  auto const flipLineIndex = ad.flipX;

  bool const gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

  float lineIndex = ad.objectData.startX ? (*ad.objectData.startX + offset) : noteData->lineIndex;
  float lineLayer = ad.objectData.startY.value_or(noteData->noteLineLayer.value__);
  float const startLineLayer = ad.startNoteLineLayer;

  //    Vector3 moveStartPos = result.moveStartPos;
  //    Vector3 moveEndPos = result.moveEndPos;
  //    Vector3 jumpEndPos = result.jumpEndPos;
  //    float jumpGravity = result.jumpGravity;

  Vector3 const noteOffset = SpawnDataHelper::GetNoteOffset(self, lineIndex, startLineLayer);

  float jumpDuration;
  float jumpDistance;
  Vector3 moveStartPos;
  Vector3 moveEndPos;
  Vector3 jumpEndPos;
  SpawnDataHelper::GetNoteJumpValues(beatmapObjectSpawnController->_initData, self, njs, spawnOffset, jumpDuration,
                                     jumpDistance, moveStartPos, moveEndPos, jumpEndPos);

  float jumpGravity;
  float noGravity;

  SpawnDataHelper::NoteJumpGravityForLineLayer(self,

                                               lineLayer,

                                               startLineLayer, jumpDistance, njs,

                                               jumpGravity, noGravity);

  float offsetStartRow = flipLineIndex.value_or(lineIndex);
  float offsetStartHeight = gravityOverride ? lineLayer : startLineLayer;

  Vector3 const noteOffset2 = SpawnDataHelper::GetNoteOffset(self, offsetStartRow, offsetStartHeight);

  auto result = BeatmapObjectSpawnMovementData::NoteSpawnData(
      moveStartPos + noteOffset2, moveEndPos + noteOffset2, jumpEndPos + noteOffset,
      gravityOverride ? noGravity : jumpGravity, self->moveDuration, jumpDuration);

  return result;
}

void InstallBeatmapObjectSpawnMovementDataHooks() {
  INSTALL_HOOK(NELogger::Logger, GetObstacleSpawnData);
  INSTALL_HOOK(NELogger::Logger, GetJumpingNoteSpawnData);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectSpawnController_Start)
}

NEInstallHooks(InstallBeatmapObjectSpawnMovementDataHooks);