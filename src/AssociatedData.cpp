#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/PointDefinition.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"

AnimationObjectData::AnimationObjectData(BeatmapAssociatedData& beatmapAD, const rapidjson::Value& animation) {
    position = AnimationHelper::TryGetPointData(beatmapAD, animation, "_position");
    rotation = AnimationHelper::TryGetPointData(beatmapAD, animation, "_rotation");
    scale = AnimationHelper::TryGetPointData(beatmapAD, animation, "_scale");
    localRotation = AnimationHelper::TryGetPointData(beatmapAD, animation, "_localRotation");
    dissolve = AnimationHelper::TryGetPointData(beatmapAD, animation, "_dissolve");
    dissolveArrow = AnimationHelper::TryGetPointData(beatmapAD, animation, "_dissolveArrow");
    cuttable = AnimationHelper::TryGetPointData(beatmapAD, animation, "_cuttable");
    definitePosition = AnimationHelper::TryGetPointData(beatmapAD, animation, "_definitePosition");
}

BeatmapAssociatedData::~BeatmapAssociatedData() {
    for (auto *pointDefinition : anonPointDefinitions) {
        delete pointDefinition;
    }
}

BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData) {
    auto itr = customData->associatedData.find('N');
    if (itr == customData->associatedData.end()) {
        itr = customData->associatedData.emplace('N', BeatmapObjectAssociatedData()).first;
    }
    return std::any_cast<BeatmapObjectAssociatedData&>(itr->second);
}

BeatmapAssociatedData& getBeatmapAD(CustomJSONData::JSONWrapper *customData) {
    auto itr = customData->associatedData.find('N');
    if (itr == customData->associatedData.end()) {
        itr = customData->associatedData.emplace('N', BeatmapAssociatedData()).first;
    }
    return std::any_cast<BeatmapAssociatedData&>(itr->second);
}