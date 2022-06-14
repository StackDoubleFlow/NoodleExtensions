#include "AssociatedData.h"

#include <utility>
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/Animation.h"
#include "NEJSON.h"

using namespace TracksAD;
using namespace NEVector;

namespace {

PointDefinition *TryGetPointData(BeatmapAssociatedData &beatmapAD,
                                 const rapidjson::Value &animation, std::string_view name) {
    PointDefinition *anonPointDef = nullptr;
    PointDefinition *pointDef =
        Animation::TryGetPointData(beatmapAD, anonPointDef, animation, name);
    if (anonPointDef) {
        beatmapAD.anonPointDefinitions.emplace(anonPointDef);
    }
    return pointDef;
}

} // namespace

AnimationObjectData::AnimationObjectData(BeatmapAssociatedData &beatmapAD, const rapidjson::Value &animation,
                                         bool v2) : parsed(true) {
    position = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_POSITION : NoodleExtensions::Constants::OFFSET_POSITION);
    rotation = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_ROTATION : NoodleExtensions::Constants::OFFSET_ROTATION);
    scale = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::SCALE);
    localRotation = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION : NoodleExtensions::Constants::LOCAL_ROTATION);
    dissolve = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_DISSOLVE : NoodleExtensions::Constants::DISSOLVE);
    dissolveArrow = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_DISSOLVE_ARROW : NoodleExtensions::Constants::DISSOLVE_ARROW);
    cuttable = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_CUTTABLE : NoodleExtensions::Constants::INTERACTABLE);
    definitePosition = TryGetPointData(beatmapAD, animation, v2 ? NoodleExtensions::Constants::V2_DEFINITE_POSITION : NoodleExtensions::Constants::DEFINITE_POSITION);
}

ObjectCustomData::ObjectCustomData(const rapidjson::Value &customData,
                                   CustomJSONData::CustomNoteData *noteData,
                                   CustomJSONData::CustomObstacleData *obstacleData, bool v2) {
    auto [x, y] = NEJSON::ReadOptionalPair(customData, v2 ? NoodleExtensions::Constants::V2_POSITION : NoodleExtensions::Constants::NOTE_OFFSET);
    startX = x;
    startY = y;
    // TODO: Mirror X

    rotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_ROTATION : NoodleExtensions::Constants::ROTATION);
    localRotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION : NoodleExtensions::Constants::LOCAL_ROTATION);
    noteJumpMovementSpeed = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_JUMP_SPEED : NoodleExtensions::Constants::NOTE_JUMP_SPEED);
    noteJumpStartBeatOffset = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_SPAWN_OFFSET : NoodleExtensions::Constants::NOTE_SPAWN_OFFSET);
    fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
    uninteractable = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_CUTTABLE : NoodleExtensions::Constants::UNINTERACTABLE);

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

    disableNoteGravity = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_GRAVITY_DISABLE : NoodleExtensions::Constants::NOTE_GRAVITY_DISABLE);
    disableNoteLook = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_LOOK_DISABLE : NoodleExtensions::Constants::NOTE_LOOK_DISABLE).value_or(false);
    scale = NEJSON::ReadOptionalScale(customData, v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::OBSTACLE_SIZE);

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

ParentTrackEventData::ParentTrackEventData(const rapidjson::Value &eventData, BeatmapAssociatedData &beatmapAD,
                                           bool v2) {
    parentTrack = &beatmapAD.tracks.try_emplace(eventData[v2 ? NoodleExtensions::Constants::V2_PARENT_TRACK.data() : NoodleExtensions::Constants::PARENT_TRACK.data()].GetString(), v2).first->second;

    rapidjson::Value const &rawChildrenTracks = eventData[v2 ? NoodleExtensions::Constants::V2_CHILDREN_TRACKS.data() : NoodleExtensions::Constants::CHILDREN_TRACKS.data()];

    childrenTracks.reserve(rawChildrenTracks.Size());
    for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin();
         itr != rawChildrenTracks.End(); itr++) {
        Track *child = &beatmapAD.tracks.try_emplace(itr->GetString(), v2).first->second;
        // NELogger::GetLogger().debug("Assigning track %s(%p) to parent track %s(%p)", itr->GetString(), child, eventData["_parentTrack"].GetString(), track);
        childrenTracks.emplace_back(child);
    }

    pos = NEJSON::ReadOptionalVector3(eventData, v2 ? NoodleExtensions::Constants::V2_POSITION : NoodleExtensions::Constants::POSITION);
    rot = NEJSON::ReadOptionalRotation(eventData, v2 ? NoodleExtensions::Constants::V2_ROTATION : NoodleExtensions::Constants::ROTATION);
    localRot = NEJSON::ReadOptionalRotation(eventData, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION : NoodleExtensions::Constants::LOCAL_ROTATION);
    scale = NEJSON::ReadOptionalVector3(eventData, v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::SCALE);

    worldPositionStays = NEJSON::ReadOptionalBool(eventData, v2 ? NoodleExtensions::Constants::V2_WORLD_POSITION_STAYS : NoodleExtensions::Constants::WORLD_POSITION_STAYS).value_or(false);
}

static std::unordered_map<CustomJSONData::CustomEventData const*, BeatmapEventAssociatedData> eventDataMap;
static std::unordered_map<GlobalNamespace::BeatmapObjectData const*, ::BeatmapObjectAssociatedData> obstacleDataMap;

::BeatmapEventAssociatedData &getEventAD(CustomJSONData::CustomEventData const* customData) {
    return eventDataMap[customData];
}

void clearEventADs() {
    eventDataMap.clear();
}

::BeatmapObjectAssociatedData &getAD(GlobalNamespace::BeatmapObjectData *objectData) {
    return obstacleDataMap[objectData];
}

void clearObjectADs() {
    obstacleDataMap.clear();
}


