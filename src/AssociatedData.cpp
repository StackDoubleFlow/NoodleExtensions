#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData) {
    return (BeatmapObjectAssociatedData *) customData->associatedData['N'];
}