#include "AssociatedData.h"
#include "Animation/AnimationHelper.h"
#include "Animation/PointDefinition.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

namespace {

PointDefinition *TryGetPointData(BeatmapAssociatedData &beatmapAD,
                                 const rapidjson::Value &animation, const char *name) {
    PointDefinition *anonPointDef;
    PointDefinition *pointDef =
        AnimationHelper::TryGetPointData(beatmapAD, anonPointDef, animation, name);
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
    cuttable = TryGetPointData(beatmapAD, animation, "_cuttable");
    definitePosition = TryGetPointData(beatmapAD, animation, "_definitePosition");
}

void BeatmapObjectAssociatedData::ResetState() {
    cutoutAnimationEffect = nullptr;
    mirroredCutoutAnimationEffect = nullptr;
    cutoutEffect = nullptr;
    mirroredCutoutEffect = nullptr;
    disappearingArrowController = nullptr;
    mirroredDisappearingArrowController = nullptr;
    materialSwitcher = nullptr;
    mirroredRenderer = nullptr;
}

BeatmapAssociatedData::~BeatmapAssociatedData() {
    for (auto *pointDefinition : anonPointDefinitions) {
        delete pointDefinition;
    }
}

BeatmapObjectAssociatedData &getAD(CustomJSONData::JSONWrapper *customData) {
    std::any &ad = customData->associatedData['N'];
    if (!ad.has_value())
        ad = std::make_any<BeatmapObjectAssociatedData>();
    return std::any_cast<BeatmapObjectAssociatedData &>(ad);
}

BeatmapAssociatedData &getBeatmapAD(CustomJSONData::JSONWrapper *customData) {
    std::any &ad = customData->associatedData['N'];
    if (!ad.has_value())
        ad = std::make_any<BeatmapAssociatedData>();
    return std::any_cast<BeatmapAssociatedData &>(ad);
}