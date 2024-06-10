#include "AssociatedData.h"

#include <utility>
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/Animation.h"
#include "NEJSON.h"

using namespace TracksAD;
using namespace NEVector;

namespace {

PointDefinition* TryGetPointData(BeatmapAssociatedData& beatmapAD, rapidjson::Value const& animation,
                                 std::string_view name) {
  PointDefinition* anonPointDef = nullptr;
  PointDefinition* pointDef = Animation::TryGetPointData(beatmapAD, anonPointDef, animation, name);
  if (anonPointDef) {
    beatmapAD.anonPointDefinitions.emplace(anonPointDef);
  }
  return pointDef;
}

} // namespace

AnimationObjectData::AnimationObjectData(BeatmapAssociatedData& beatmapAD, rapidjson::Value const& animation, bool v2)
    : parsed(true) {
  position =
      TryGetPointData(beatmapAD, animation,
                      v2 ? NoodleExtensions::Constants::V2_POSITION : NoodleExtensions::Constants::OFFSET_POSITION);
  rotation =
      TryGetPointData(beatmapAD, animation,
                      v2 ? NoodleExtensions::Constants::V2_ROTATION : NoodleExtensions::Constants::OFFSET_ROTATION);
  scale = TryGetPointData(beatmapAD, animation,
                          v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::SCALE);
  localRotation = TryGetPointData(beatmapAD, animation,
                                  v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION
                                     : NoodleExtensions::Constants::LOCAL_ROTATION);
  dissolve = TryGetPointData(beatmapAD, animation,
                             v2 ? NoodleExtensions::Constants::V2_DISSOLVE : NoodleExtensions::Constants::DISSOLVE);
  dissolveArrow = TryGetPointData(beatmapAD, animation,
                                  v2 ? NoodleExtensions::Constants::V2_DISSOLVE_ARROW
                                     : NoodleExtensions::Constants::DISSOLVE_ARROW);
  cuttable = TryGetPointData(beatmapAD, animation,
                             v2 ? NoodleExtensions::Constants::V2_CUTTABLE : NoodleExtensions::Constants::INTERACTABLE);
  definitePosition = TryGetPointData(beatmapAD, animation,
                                     v2 ? NoodleExtensions::Constants::V2_DEFINITE_POSITION
                                        : NoodleExtensions::Constants::DEFINITE_POSITION);
}

ObjectCustomData::ObjectCustomData(rapidjson::Value const& customData, CustomJSONData::CustomNoteData* noteData,
                                   CustomJSONData::CustomObstacleData* obstacleData, bool v2) {
  auto [x, y] = NEJSON::ReadOptionalPair(customData, v2 ? NoodleExtensions::Constants::V2_POSITION
                                                        : NoodleExtensions::Constants::NOTE_OFFSET);
  auto [tailX, tailY] = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::TAIL_NOTE_OFFSET);
  startX = x;
  startY = y;

  tailStartX = tailX;
  tailStartY = tailY;

  // TODO: Mirror X

  rotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_ROTATION
                                                         : NoodleExtensions::Constants::WORLD_ROTATION);
  localRotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION
                                                              : NoodleExtensions::Constants::LOCAL_ROTATION);
  noteJumpMovementSpeed = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_JUMP_SPEED
                                                                   : NoodleExtensions::Constants::NOTE_JUMP_SPEED);
  noteJumpStartBeatOffset = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_SPAWN_OFFSET
                                                                     : NoodleExtensions::Constants::NOTE_SPAWN_OFFSET);
  fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE
                                                 : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
  uninteractable = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_CUTTABLE
                                                           : NoodleExtensions::Constants::UNINTERACTABLE);

  uninteractable = v2 ? !uninteractable.value_or(true) : uninteractable;

  if (noteData && fake.value_or(false)) {
    noteData->set_scoringType(GlobalNamespace::NoteData::ScoringType::NoScore);
  }

  if (v2) {
    auto cutDirOpt = NEJSON::ReadOptionalFloat(customData, NoodleExtensions::Constants::V2_CUT_DIRECTION);

    if (cutDirOpt) {
      // TODO: MIRROR
      //            noteData->SetCutDirectionAngleOffset(Mirror(*cutDirOpt, leftHanded));
      noteData->SetCutDirectionAngleOffset(*cutDirOpt);
      if (noteData->cutDirection != GlobalNamespace::NoteCutDirection::Any) {
        noteData->ChangeNoteCutDirection(GlobalNamespace::NoteCutDirection::Down);
      }
    }
  }

  // TODO: FLIP X

  disableNoteGravity = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_GRAVITY_DISABLE
                                                               : NoodleExtensions::Constants::NOTE_GRAVITY_DISABLE);
  disableNoteLook = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_LOOK_DISABLE
                                                            : NoodleExtensions::Constants::NOTE_LOOK_DISABLE)
                        .value_or(false);
  scale = NEJSON::ReadOptionalScale(customData, v2 ? NoodleExtensions::Constants::V2_SCALE
                                                   : NoodleExtensions::Constants::OBSTACLE_SIZE);

  link = NEJSON::ReadOptionalString(customData, NoodleExtensions::Constants::LINK);

  // TODO: MIRROR WIDTH AND OBSTACLE START X

  //    float width = obstacleData->width;
  //    if (scale) {
  //        width = (*scale)[0].value_or(width);
  //    }
  //
  //    if (startX) {
  //        startX = (*startX + width) * -1;
  //    } else if (scale && (*scale)[0]){
  //        auto lineIndex = obstacleData->lineIndex - 2;
  //        (*scale)[0] = (lineIndex + width) * -1;
  //    }
}

void ::BeatmapObjectAssociatedData::ResetState() {
  parsed = false;
}

ParentTrackEventData::ParentTrackEventData(rapidjson::Value const& eventData, BeatmapAssociatedData& beatmapAD,
                                           bool v2) {
  parentTrack = beatmapAD.getTrack(eventData[v2 ? NoodleExtensions::Constants::V2_PARENT_TRACK.data()
                                                : NoodleExtensions::Constants::PARENT_TRACK.data()]
                                       .GetString());

  rapidjson::Value const& rawChildrenTracks = eventData[v2 ? NoodleExtensions::Constants::V2_CHILDREN_TRACKS.data()
                                                           : NoodleExtensions::Constants::CHILDREN_TRACKS.data()];

  if (rawChildrenTracks.IsArray()) {
    childrenTracks.reserve(rawChildrenTracks.Size());
    for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin(); itr != rawChildrenTracks.End(); itr++) {
      Track* child = beatmapAD.getTrack(itr->GetString());
      // NELogger::Logger.debug("Assigning track {}(%p) to parent track {}(%p)", itr->GetString(), child,
      // eventData["_parentTrack"].GetString(), track);
      childrenTracks.emplace_back(child);
    }
  } else {
    childrenTracks = { beatmapAD.getTrack(rawChildrenTracks.GetString()) };
  }

  pos = NEJSON::ReadOptionalVector3(eventData, v2 ? NoodleExtensions::Constants::V2_POSITION
                                                  : NoodleExtensions::Constants::OFFSET_POSITION);
  localPos = NEJSON::ReadOptionalVector3(eventData, v2 ? NoodleExtensions::Constants::V2_LOCAL_POSITION
                                                       : NoodleExtensions::Constants::LOCAL_POSITION);
  rot = NEJSON::ReadOptionalRotation(eventData, v2 ? NoodleExtensions::Constants::V2_ROTATION
                                                   : NoodleExtensions::Constants::WORLD_ROTATION);
  localRot = NEJSON::ReadOptionalRotation(eventData, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION
                                                        : NoodleExtensions::Constants::LOCAL_ROTATION);
  scale = NEJSON::ReadOptionalVector3(eventData,
                                      v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::SCALE);

  worldPositionStays = NEJSON::ReadOptionalBool(eventData, v2 ? NoodleExtensions::Constants::V2_WORLD_POSITION_STAYS
                                                              : NoodleExtensions::Constants::WORLD_POSITION_STAYS)
                           .value_or(false);
}

PlayerTrackEventData::PlayerTrackEventData(Track* track, std::optional<std::string_view> targetOpt) : track(track) {
  this->target = PlayerTrackObject::ENTIRE_PLAYER;

  if (targetOpt) {
    auto targetStr = *targetOpt;

    if (targetStr == "ENTIRE_PLAYER") {
      this->target = PlayerTrackObject::ENTIRE_PLAYER;
    }
    if (targetStr == "HMD") {
      this->target = PlayerTrackObject::HMD;
    }
    if (targetStr == "LEFT_HAND") {
      this->target = PlayerTrackObject::LEFT_HAND;
    }
    if (targetStr == "RIGHT_HAND") {
      this->target = PlayerTrackObject::RIGHT_HAND;
    }
  }
}

static std::unordered_map<CustomJSONData::CustomEventData const*, BeatmapEventAssociatedData> eventDataMap;
static std::unordered_map<GlobalNamespace::BeatmapObjectData const*, ::BeatmapObjectAssociatedData> obstacleDataMap;

::BeatmapEventAssociatedData& getEventAD(CustomJSONData::CustomEventData const* customData) {
  return eventDataMap[customData];
}

void clearEventADs() {
  eventDataMap.clear();
}

::BeatmapObjectAssociatedData& getAD(GlobalNamespace::BeatmapObjectData* objectData) {
  return obstacleDataMap[objectData];
}

void clearObjectADs() {
  obstacleDataMap.clear();
}
