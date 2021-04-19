#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"

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