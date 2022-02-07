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
    PointDefinition *anonPointDef;
    PointDefinition *pointDef =
        Animation::TryGetPointData(beatmapAD, anonPointDef, animation, name);
    if (anonPointDef) {
        beatmapAD.anonPointDefinitions.push_back(anonPointDef);
    }
    return pointDef;
}

} // namespace

AnimationObjectData::AnimationObjectData(BeatmapAssociatedData &beatmapAD,
                                         const rapidjson::Value &animation) : parsed(true) {
    position = TryGetPointData(beatmapAD, animation, "_position");
    rotation = TryGetPointData(beatmapAD, animation, "_rotation");
    scale = TryGetPointData(beatmapAD, animation, "_scale");
    localRotation = TryGetPointData(beatmapAD, animation, "_localRotation");
    dissolve = TryGetPointData(beatmapAD, animation, "_dissolve");
    dissolveArrow = TryGetPointData(beatmapAD, animation, "_dissolveArrow");
    cuttable = TryGetPointData(beatmapAD, animation, "_interactable");
    definitePosition = TryGetPointData(beatmapAD, animation, "_definitePosition");
}

ObjectCustomData::ObjectCustomData(const rapidjson::Value &customData, std::optional<NEVector::Vector2>& flip) {
    auto [x, y] = NEJSON::ReadOptionalPair(customData, "_position");
    startX = x;
    startY = y;

    rotation = NEJSON::ReadOptionalRotation(customData, "_rotation");
    localRotation = NEJSON::ReadOptionalRotation(customData, "_localRotation");
    noteJumpMovementSpeed = NEJSON::ReadOptionalFloat(customData, "_noteJumpMovementSpeed");
    noteJumpStartBeatOffset = NEJSON::ReadOptionalFloat(customData, "_noteJumpStartBeatOffset");
    fake = NEJSON::ReadOptionalBool(customData, "_fake");
    interactable = NEJSON::ReadOptionalBool(customData, "_interactable");
    auto cutDirOpt = NEJSON::ReadOptionalFloat(customData, "_cutDirection");

    if (cutDirOpt)
        cutDirection = NEVector::Quaternion::Euler(0, 0, cutDirOpt.value());

    auto newFlip = NEJSON::ReadOptionalVector2_emptyY(customData, "_flip");
    if (newFlip)
        flip = newFlip;

    disableNoteGravity = NEJSON::ReadOptionalBool(customData, "_disableNoteGravity");
    disableNoteLook = NEJSON::ReadOptionalBool(customData, "_disableNoteLook").value_or(false);
    scale = NEJSON::ReadOptionalScale(customData, "_scale");
}

void ::BeatmapObjectAssociatedData::ResetState() {
    cutoutAnimationEffect = nullptr;
    mirroredCutoutAnimationEffect = nullptr;
    cutoutEffect = nullptr;
    mirroredCutoutEffect = nullptr;
    disappearingArrowController = nullptr;
    mirroredDisappearingArrowController = nullptr;
    materialSwitchers = nullptr;
    parsed = false;
}

ParentTrackEventData::ParentTrackEventData(const rapidjson::Value &eventData, BeatmapAssociatedData &beatmapAD) {
    parentTrack = &beatmapAD.tracks[eventData["_parentTrack"].GetString()];

    rapidjson::Value const &rawChildrenTracks = eventData["_childrenTracks"];

    childrenTracks.reserve(rawChildrenTracks.Size());
    for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin();
         itr != rawChildrenTracks.End(); itr++) {
        Track *child = &beatmapAD.tracks[itr->GetString()];
        // NELogger::GetLogger().debug("Assigning track %s(%p) to parent track %s(%p)", itr->GetString(), child, eventData["_parentTrack"].GetString(), track);
        childrenTracks.emplace_back(child);
    }

    pos = NEJSON::ReadOptionalVector3(eventData, "_position");
    rot = NEJSON::ReadOptionalRotation(eventData, "_rotation");
    localRot = NEJSON::ReadOptionalRotation(eventData, "_localRotation");
    scale = NEJSON::ReadOptionalVector3(eventData, "_scale");

    worldPositionStays = NEJSON::ReadOptionalBool(eventData, "_worldPositionStays").value_or(false);
}

::BeatmapObjectAssociatedData &getAD(CustomJSONData::JSONWrapper *customData) {
    std::any &ad = customData->associatedData['N'];
    if (!ad.has_value())
        ad = std::make_any<::BeatmapObjectAssociatedData>();
    return std::any_cast<::BeatmapObjectAssociatedData &>(ad);
}

static std::unordered_map<CustomJSONData::CustomEventData const*, BeatmapEventAssociatedData> eventDataMap;

::BeatmapEventAssociatedData &getEventAD(CustomJSONData::CustomEventData const* customData) {
    return eventDataMap[customData];
}

void clearEventADs() {
    eventDataMap.clear();
}


