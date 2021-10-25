#include "AssociatedData.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/Animation.h"
#include "NEJSON.h"

using namespace TracksAD;

namespace {

PointDefinition *TryGetPointData(BeatmapAssociatedData &beatmapAD,
                                 const rapidjson::Value &animation, const char *name) {
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
                                         const rapidjson::Value &animation) {
    position = TryGetPointData(beatmapAD, animation, "_position");
    rotation = TryGetPointData(beatmapAD, animation, "_rotation");
    scale = TryGetPointData(beatmapAD, animation, "_scale");
    localRotation = TryGetPointData(beatmapAD, animation, "_localRotation");
    dissolve = TryGetPointData(beatmapAD, animation, "_dissolve");
    dissolveArrow = TryGetPointData(beatmapAD, animation, "_dissolveArrow");
    cuttable = TryGetPointData(beatmapAD, animation, "_interactable");
    definitePosition = TryGetPointData(beatmapAD, animation, "_definitePosition");
}

ObjectCustomData::ObjectCustomData(const rapidjson::Value &customData) {
    position = NEJSON::ReadOptionalVector2(customData, "_position");
    rotation = NEJSON::ReadOptionalRotation(customData, "_rotation");
    localRotation = NEJSON::ReadOptionalRotation(customData, "_localRotation");
    noteJumpMovementSpeed = NEJSON::ReadOptionalFloat(customData, "_noteJumpMovementSpeed");
    noteJumpStartBeatOffset = NEJSON::ReadOptionalFloat(customData, "_noteJumpStartBeatOffset");
    fake = NEJSON::ReadOptionalBool(customData, "_fake");
    interactable = NEJSON::ReadOptionalBool(customData, "_interactable");
    auto cutDirOpt = NEJSON::ReadOptionalFloat(customData, "_cutDirection");

    if (cutDirOpt)
        cutDirection = NEVector::Quaternion::Euler(0, 0, cutDirOpt.value());

    flip = NEJSON::ReadOptionalVector2_emptyY(customData, "_flip");
    disableNoteGravity = NEJSON::ReadOptionalBool(customData, "_disableNoteGravity");
    disableNoteLook = NEJSON::ReadOptionalBool(customData, "_disableNoteLook");
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
}

::BeatmapObjectAssociatedData &getAD(CustomJSONData::JSONWrapper *customData) {
    std::any &ad = customData->associatedData['N'];
    if (!ad.has_value())
        ad = std::make_any<::BeatmapObjectAssociatedData>();
    return std::any_cast<::BeatmapObjectAssociatedData &>(ad);
}