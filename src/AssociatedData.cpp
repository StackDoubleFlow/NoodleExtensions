#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/PointDefinition.h"
#include "AssociatedData.h"

BeatmapAssociatedData::~BeatmapAssociatedData() {
    for (auto *pointDefinition : anonPointDefinitions) {
        delete pointDefinition;
    }
}

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData) {
    if (customData->associatedData.find('N') == customData->associatedData.end()) {
        customData->associatedData['N'] = new BeatmapObjectAssociatedData();
    }
    return (BeatmapObjectAssociatedData *) customData->associatedData['N'];
}

BeatmapAssociatedData *getBeatmapAD(CustomJSONData::JSONWrapper *customData) {
    return (BeatmapAssociatedData *) customData->associatedData['N'];
}