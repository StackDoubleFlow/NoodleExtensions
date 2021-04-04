#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData) {
    if (customData->associatedData.find('N') == customData->associatedData.end()) {
        customData->associatedData['N'] = new BeatmapObjectAssociatedData();
    }
    return (BeatmapObjectAssociatedData *) customData->associatedData['N'];
}

BeatmapAssociatedData *getBeatmapAD(CustomJSONData::JSONWrapper *customData) {
    return (BeatmapAssociatedData *) customData->associatedData['N'];
}